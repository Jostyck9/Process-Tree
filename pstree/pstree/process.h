#pragma once

#include <windows.h>
#include <list>
#include <memory>

typedef struct sProcess Process;
typedef struct sProcess {
	WCHAR name[MAX_PATH];
	DWORD id;
	DWORD parentId;
	std::list<std::shared_ptr<Process>> children;
} Process;