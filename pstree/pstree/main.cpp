#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <memory>

#include "process.h"

const std::string INDENT = "|   ";

/**
* Print the info from a process like process name, id and parent id
*
* @param process Process structur shared ptr
**/
void printProcessInfo(std::shared_ptr<Process>& process) {
	_tprintf(TEXT("+-%s (%lu) (%lu)\n"), process->name, process->id, process->parentId);
}

/**
* Print the process tree
*
* @param rootProcess Process structur shared ptr root node
* @param indent level of indentation to display
**/
void printTree(std::shared_ptr<Process>& rootProcess, unsigned int indent) {
	std::string startingIndent = "";

	for (unsigned int i = 0; i < indent; i++) {
		startingIndent += INDENT;
	}
	printf("%s", startingIndent.c_str());
	printProcessInfo(rootProcess);
	for (auto& child : rootProcess->children) {
		printTree(child, indent + 1);
	}
}

/**
* Convert a ProcessEntry32 struct to a Custom process node
*
* @param lppe ptr of the ProcessEntry32 to convert
* @return shared_ptr<Process>
**/
std::shared_ptr<Process> convertToProcess(LPPROCESSENTRY32 lppe) {
	std::shared_ptr<Process> toReturn = std::make_shared<Process>();

	wcscpy_s(toReturn->name, lppe->szExeFile);
	toReturn->id = lppe->th32ProcessID;
	toReturn->parentId = lppe->th32ParentProcessID;
	return toReturn;
}

/**
* Add a process node to the list of process and in the tree hierarchy
*
* @param listProcess list of process where to add new node
* @param process Process to add
**/
void addToProcessParent(std::list<std::shared_ptr<Process>>& listProcess, std::shared_ptr<Process> process) {
	bool add = false;
	for (auto& current : listProcess) {
		if (current->id == process->parentId) {
			current->children.push_back(process);
			add = true;
			break;
		}
	}
	listProcess.push_back(process);
	if (!add && process->id != 0) {
		listProcess.front()->children.push_back(process);
	}
}

int main(int argc, char argv[]) {
	HANDLE hSnapshot;
	PROCESSENTRY32 processEntry;
	std::list<std::shared_ptr<Process>> listProcess;
	
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		std::cerr << "An error while taking snapshot of the system" << std::endl;
		return 84;
	}

	if (Process32First(hSnapshot, &processEntry)) {
		addToProcessParent(listProcess, convertToProcess(&processEntry));
		while (Process32Next(hSnapshot, &processEntry)) {
			addToProcessParent(listProcess, convertToProcess(&processEntry));
		}
	}
	CloseHandle(hSnapshot);
	if (GetLastError() != ERROR_NO_MORE_FILES) {
		std::cerr << "An error occurred while reading process info" << std::endl;
		return 84;
	}
	_tprintf(TEXT("Number of Running Processes = %i\n"), listProcess.size());
	printTree(listProcess.front(), 0);
	return 0;
}