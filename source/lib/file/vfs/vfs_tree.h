/* Copyright (c) 2010 Wildfire Games
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * 'tree' of VFS directories and files
 */

#ifndef INCLUDED_VFS_TREE
#define INCLUDED_VFS_TREE

#include <map>

#include "lib/file/file_system.h"	// FileInfo
#include "lib/file/common/file_loader.h"	// PIFileLoader
#include "lib/file/common/real_directory.h"	// PRealDirectory

class VfsFile
{
public:
	VfsFile(const std::wstring& name, size_t size, time_t mtime, size_t priority, const PIFileLoader& provider);

	const std::wstring& Name() const
	{
		return m_name;
	}

	size_t Size() const
	{
		return m_size;
	}

	time_t MTime() const
	{
		return m_mtime;
	}

	size_t Priority() const
	{
		return m_priority;
	}

	const PIFileLoader& Loader() const
	{
		return m_loader;
	}

private:
	std::wstring m_name;
	size_t m_size;
	time_t m_mtime;

	size_t m_priority;

	PIFileLoader m_loader;
};


class VfsDirectory
{
public:
	typedef std::map<std::wstring, VfsFile> VfsFiles;
	typedef std::map<std::wstring, VfsDirectory> VfsSubdirectories;

	VfsDirectory();

	/**
	 * @return address of existing or newly inserted file.
	 **/
	VfsFile* AddFile(const VfsFile& file);

	/**
	 * @return address of existing or newly inserted subdirectory.
	 **/
	VfsDirectory* AddSubdirectory(const std::wstring& name);

	/**
	 * @return file with the given name.
	 * (note: non-const to allow changes to the file)
	 **/
	VfsFile* GetFile(const std::wstring& name);

	/**
	 * @return subdirectory with the given name.
	 * (note: non-const to allow changes to the subdirectory)
	 **/
	VfsDirectory* GetSubdirectory(const std::wstring& name);

	// note: exposing only iterators wouldn't enable callers to reserve space.

	const VfsFiles& Files() const
	{
		return m_files;
	}

	const VfsSubdirectories& Subdirectories() const
	{
		return m_subdirectories;
	}

	/**
	 * side effect: the next ShouldPopulate() will return true.
	 **/
	void SetAssociatedDirectory(const PRealDirectory& realDirectory);

	const PRealDirectory& AssociatedDirectory() const
	{
		return m_realDirectory;
	}

	/**
	 * @return whether this directory should be populated from its
	 * AssociatedDirectory(). note that calling this is a promise to
	 * do so if true is returned -- the flag is reset immediately.
	 **/
	bool ShouldPopulate();

	/**
	 * indicate that a file has changed; ensure its new version supersedes
	 * the old by removing it and marking the directory for re-population.
	 **/
	void Invalidate(const std::wstring& name);

	/**
	 * empty file and subdirectory lists (e.g. when rebuilding VFS).
	 * CAUTION: this invalidates all previously returned pointers.
	 **/
	void Clear();

private:
	VfsFiles m_files;
	VfsSubdirectories m_subdirectories;

	PRealDirectory m_realDirectory;
	volatile intptr_t m_shouldPopulate;	// (cpu_CAS can't be used on bool)
};


/**
 * @return a string containing file attributes (location, size, timestamp) and name.
 **/
extern std::wstring FileDescription(const VfsFile& file);

/**
 * @return a string holding each files' description (one per line).
 **/
extern std::wstring FileDescriptions(const VfsDirectory& directory, size_t indentLevel);

/**
 * append each directory's files' description to the given string.
 **/
void DirectoryDescriptionR(std::wstring& descriptions, const VfsDirectory& directory, size_t indentLevel);

#endif	// #ifndef INCLUDED_VFS_TREE
