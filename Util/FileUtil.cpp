/*
 * FileUtil.cpp
 *
 *  Created on: Jan. 31, 2021
 *      Author: blackaddr
 */
#include <JuceHeader.h>
#include "Util/CommonDefs.h"
#include "Util/ErrorMessage.h"
#include "Util/ErrorMessageWindow.h"
#include "Util/StringUtil.h"
#include "Util/FileUtil.h"

using namespace juce;

namespace stride {
// This file implements file I/O parsing using the JUCE libraries.

std::string FileUtil::getSystemTempDirectory()
{
    return File::getSpecialLocation(File::SpecialLocationType::tempDirectory).getFullPathName().toStdString();
}

std::string FileUtil::fileSeparator()
{
    return String(File::getSeparatorString()).toStdString();
}

int FileUtil::writeStringToFile(const std::string& str, const std::string& filename, bool appendToFile)
{
    if (!isAbsolutePath(filename)) { return FAILURE; }
    String juceFilename = String(filename);

    File file(juceFilename);
    FileOutputStream fileOutputStream(file); // Note, default implies file cannot exceed 16B
    if (fileOutputStream.openedOk()) {
        if (!appendToFile) {
            fileOutputStream.setPosition(0); // set position to zero, then truncate to overwrite
            fileOutputStream.truncate();
        }
        fileOutputStream.writeString(String(str));

        // This also writes a null terminator to the file, which is incorrect. We have to backup
        // one byte and remove it.
        fileOutputStream.setPosition(fileOutputStream.getPosition()-1);
        fileOutputStream.truncate();
        return 0;
    } else {
        // TODO: handle error
        return -1;
    }
}

int FileUtil::writeStringToFile(const void* dataPtr, size_t numBytes, const std::string& filename, bool appendToFile)
{
    if (!isAbsolutePath(filename)) { return FAILURE; }
    String juceFilename = String(filename);

    File file(juceFilename);
    FileOutputStream fileOutputStream(file); // Note, default implies file cannot exceed 16B
    if (fileOutputStream.openedOk()) {
        if (!appendToFile) {
            fileOutputStream.setPosition(0); // set position to zero, then truncate to overwrite
            fileOutputStream.truncate();
        }
        fileOutputStream.write(dataPtr, numBytes);

        return SUCCESS;
    } else {
        // TODO: handle error
        return FAILURE;
    }
}

int FileUtil::readFileToString(const std::string& filename, std::string& strOutput)
{
    if (!isAbsolutePath(filename)) { return FAILURE; }
    String juceFilename = String(filename);

    File inputFile(juceFilename);
    if (!inputFile.existsAsFile()) {
        return FAILURE;
    }

    String juceStr = inputFile.loadFileAsString();
    strOutput = juceStr.toStdString();
    return SUCCESS;
}

bool FileUtil::directoryExists(const std::string& directory)
{
    if (!isAbsolutePath(directory)) { return FAILURE; }
    File file = File(directory);
    return file.isDirectory();
}

int FileUtil::createDirectory(const std::string& directory)
{
    if (!isAbsolutePath(directory)) { return FAILURE; }

    File newDir = File(String(directory));
    Result result = newDir.createDirectory();
    if (result.failed()) {
        errorMessage(std::string("FileUtil::createDirectory(): Failed to create directory named:" + newDir.getFullPathName().toStdString()));
        return FAILURE;
    }
    return SUCCESS;
}

int  FileUtil::createDirectoryConditionally(const std::string& directory)
{
    if (!isAbsolutePath(directory)) { return FAILURE; }
    if (directoryExists(directory)) { return SUCCESS; }

    return createDirectory(directory);
}

int FileUtil::deleteDirectory(const std::string& directory)
{
    if (!isAbsolutePath(directory)) { return FAILURE; }

    File newDir = File(String(directory));
    bool isSuccess = newDir.deleteRecursively();
    if (!isSuccess) {
        errorMessage(std::string("Failed to delete directory named:" + newDir.getFullPathName().toStdString()));
        return -1;
    }
    return SUCCESS;
}

int FileUtil::deleteDirectoryIfExists(const std::string& directory)
{
    if (!isAbsolutePath(directory)) { return SUCCESS; }

    File dirToDelete = File(String(directory));
    if (dirToDelete.isDirectory()) {
        bool isSuccess = dirToDelete.deleteRecursively();
        if (!isSuccess) {
            errorMessage(std::string("FileUtil::deleteDirectoryIfExists(): Failed to delete directory named:" + dirToDelete.getFullPathName().toStdString()));
            return FAILURE;
        }
    }
    return SUCCESS;
}

int FileUtil::copyDirectoryFromTo(const std::string& srcDir, const std::string& destDir)
{

    File srcDirFile;
    if (!isAbsolutePath(srcDir)) { return FAILURE; }
    else { srcDirFile = File(srcDir); }

    File destDirFile;
    if (!isAbsolutePath(destDir)) { return FAILURE; }
    else { destDirFile = File(destDir); }

    if (!srcDirFile.isDirectory()) {
        std::string errMsg = "FileUtil::copyDirectoryFromTo(): invalid src directory : " + srcDirFile.getFullPathName().toStdString();
        errorMessage(errMsg);
        return FAILURE;
    }

    if (!srcDirFile.copyDirectoryTo(destDirFile)) {  // copyDirectoryTo() is recursive
        std::string errMsg = "FileUtil::copyDirectoryFromTo(): failed to copy from " +
        srcDirFile.getFullPathName().toStdString() + " to  " + destDirFile.getFullPathName().toStdString();
        errorMessage(errMsg);
        return FAILURE;
    }
    return SUCCESS;
}

std::string FileUtil::getParentDir(const std::string filepath)
{
    if (!isAbsolutePath(filepath)) { return std::string(); }

    File filepathFile = File(filepath);
    return filepathFile.getParentDirectory().getFullPathName().toStdString();
}

std::string FileUtil::getChildDirs(const std::string& filePath, const std::string& parentDir)
{
    if (parentDir.empty()) { return filePath; }
    std::string parentDirStr = parentDir;  // make a copy for potential modifications
    std::string filePathStr  = filePath;
    if (parentDir.back() != fileSeparator()) { parentDirStr += fileSeparator();}  // add trailing separator if needed

    std::string::size_type startPos = filePathStr.find(parentDirStr);

    if (startPos != std::string::npos) {
        filePathStr.erase(startPos, parentDirStr.length());
    }
    return filePathStr;
}

int FileUtil::copyFile(const std::string& destFilemame, const std::string& srcFilename)
{
    if (!isAbsolutePath(destFilemame)) { return FAILURE; }
    if (!isAbsolutePath(srcFilename))  { return FAILURE; }

    File src = File(srcFilename);
    File dest = File(destFilemame);

    noteMessage(std::string("FileUtil::copyFile(): Copying " + srcFilename + " to " + destFilemame));

    // Copy the file to the effects folder
    bool isSuccess = src.copyFileTo(dest);

    return (isSuccess ? 0 : -1);
}

int FileUtil::copyFileToDir(const std::string& destDir, const std::string& srcFilename)
{
    if (!isAbsolutePath(srcFilename))  { return FAILURE; }
    if (!isAbsolutePath(destDir))  { return FAILURE; }
    File src = File(srcFilename);
    File dest = File(String(destDir) + File::getSeparatorString() +  src.getFileName());

    noteMessage(std::string("FileUtil::copyFileToDir(): Copying " + srcFilename + " to " + dest.getFullPathName().toStdString()));

    // Copy the file to the effects folder
    bool isSuccess = src.copyFileTo(dest);

    return (isSuccess ? 0 : -1);
}

int FileUtil::copyFileWithSubDirsToDir(const std::string& destRootDir, const std::string srcRootDir, const std::string& srcFilename)
{
    // replace(full target dir, srcRootDir, destRootDir)
    std::string partialPath = srcFilename;  // init to full src path
    if (!StringUtil::replace(partialPath, srcRootDir, std::string())) {
        errorMessage("FileUtil::copyFileWithSubDirsToDir(): can't find srcRootDir in srcFilename");
        return -1;
    }
    // partialPath now is the original filename with the srcRootDir removed

    int result;

    // create the destination full path by appending the partialPath onto the destination root dir
    std::string destFile = destRootDir + partialPath;  // srcFile should already have a leading separator
    std::string destFileDirStr = FileUtil::getParentDir(destFile);
    result = FileUtil::createDirectory(destFileDirStr);
    if (result != SUCCESS) {
        std::string msg = "FileUtil::copyFileWithSubDirsToDir(): unable to create directory " + destFileDirStr;
        errorMessage(msg);
        return -1;
    }

    result = FileUtil::copyFileToDir(destFileDirStr, srcFilename);

    if (result != SUCCESS) {
        std::string msg = "FileUtil::copyFileWithSubDirsToDir(): unable to copy " + srcFilename + " to " + destFileDirStr;
        errorMessage(msg);
        return -1;
    }
    return SUCCESS;
}

int FileUtil::copyDirectoryFromInTo(const std::string& srcDir, const std::string& destDir)
{
    std::string destDirFull = destDir + fileSeparator() + File(srcDir).getFileName().toStdString();
    if (directoryExists(srcDir)) {
        return copyDirectoryFromTo(srcDir, destDirFull);
    } else {
        std::string msg = "FileUtil::copyDirectoryFromInTo(): unable to copy " + srcDir + " to " + destDirFull;
        errorMessage(msg);
        return FAILURE;
    }
}

int FileUtil::createParentDirectoryTree(const std::string& directoryPath)
{
    if (!isAbsolutePath(directoryPath))  { return FAILURE; }
    File file = File(directoryPath);
    Result res = file.createDirectory();
    if (res.failed()) { return FAILURE; }
    else { return SUCCESS; }
}

int FileUtil::deleteFile(const std::string& filename)
{
    if (!isAbsolutePath(filename)) { return FAILURE; }

    File file = File(filename);
    bool isSuccess = file.deleteFile();
    return (isSuccess ? 0 : -1);
}

int FileUtil::deleteFilePattern(const std::string& directory, const std::string& filePattern)
{
    int result = SUCCESS;
    if (!isAbsolutePath(directory)) { return FAILURE; }

    File dirFile = File(directory);
    Array<File> filesToDeleteList = dirFile.findChildFiles(File::findFilesAndDirectories, true, filePattern);
    for (auto file : filesToDeleteList) {
        std::string msg = "FileUtil::deleteFilePattern(): DELETING " + file.getFullPathName().toStdString();
        noteMessage(msg);
        if (!file.deleteRecursively()) {
            std::string msg = "FileUtil::deleteFilePattern(): failed to delete " + file.getFullPathName().toStdString();
            errorMessage(msg);
            result = FAILURE;
        }
    }
    return result;
}

bool FileUtil::fileExists(const std::string& filename)
{
    if (!isAbsolutePath(filename)) { return false; }

    File file = File(filename);
    return file.existsAsFile();
}

size_t FileUtil::getFileSize(const std::string& filename)
{
    if (!isAbsolutePath(filename)) { return false; }
    File file = File(filename);
    bool fileExists = file.existsAsFile();
    if (!fileExists) { return 0; }
    size_t siz = static_cast<size_t>(file.getSize());
    return siz;
}

int FileUtil::deleteFileIfExists(const std::string& filename)
{
    if (!isAbsolutePath(filename)) { return FAILURE; }

    if (fileExists(filename)) {
        return deleteFile(filename);
    } else {
        return SUCCESS;
    }
}

std::string FileUtil::getFilenameFromPath(const std::string& filenamePath)
{
    if (!isAbsolutePath(filenamePath)) { return std::string(); }

    File filepath = File(String(filenamePath));
    return filepath.getFileName().toStdString();
}

std::string FileUtil::getFilenameFromRelativePath(const std::string& filenamePath)
{
    // Turn the relative path into a fake absolute path so we can use the File convenience functions
#ifdef LINUX
    std::string fakeAbsolutePath = fileSeparator() + filenamePath;
#else
//#error Unsupported OS in FileUtil::getFilenameFromRelativePath)
	std::string fakeAbsolutePath = std::string("C:\\" + filenamePath);
#endif
    File filepath = File(String(fakeAbsolutePath));
    return filepath.getFileName().toStdString();
}

bool FileUtil::isAbsolutePath(const std::string& filenamePath)
{
    if (filenamePath.empty()) { return false; }
    String fullPathname = String(filenamePath);

	// TODO use proper OS level checks
	if (fullPathname.startsWith("/")) {
		return true;
	} else if (fullPathname.contains(":\\")) {
		return true;
	}
	else if (fullPathname.contains(":/")) {
		return true;
    } else {
        return false;
    }
}

std::string FileUtil::convertIfRelativePath(const std::string& filenamePath)
{
    return File::getCurrentWorkingDirectory().getChildFile(filenamePath).getFullPathName().toStdString();
}

std::string FileUtil::getFilenameExtension(const std::string& filenamePath)
{
    if (!isAbsolutePath(filenamePath)) { return std::string(); }

    File filepath = File(String(filenamePath));
    return filepath.getFileExtension().toStdString();
}

std::string FileUtil::getFilenameWithoutExtension(const std::string& filenamePath)
{
    // if the filename has no extension, return the original name
    if (filenamePath.find(".") == std::string::npos) {
        // no extension, return the original filename
        return filenamePath;
    }
    File filepath = File::createFileWithoutCheckingPath(String(filenamePath));

    return filepath.getFileNameWithoutExtension().toStdString();
}

std::string FileUtil::getFolderFromPath(const std::string& filepath)
{
    File file = File::createFileWithoutCheckingPath(String(filepath));
    return file.getParentDirectory().getFullPathName().toStdString();
}

int FileUtil::createZipfileFromDir(const std::string& zipfilePath, const std::string& srcDirectory)
{
    if (!directoryExists(srcDirectory)) {
        errorMessage(std::string("FileUtil::createZipfileFromDir(): srcDirectory " + srcDirectory + " does not exist"));
        return FAILURE;
    }

    ZipFile::Builder zipBuilder;
    Array<File> tempFiles;
    File tempFolder = File(String(srcDirectory));
    File outputZip = File(String(zipfilePath));

    tempFolder.findChildFiles(tempFiles, File::findFiles, true, "*");

    // add files
    for (int i = 0; i < tempFiles.size(); i++)
    {
        zipBuilder.addFile(tempFiles[i], 6 /* compression 0 to 9 */, tempFiles[i].getRelativePathFrom(tempFolder));
    }

    //save our zip file
    double *progress = nullptr;
    if (outputZip.exists())
    {
        outputZip.deleteFile();
    }
    FileOutputStream os (outputZip);
    zipBuilder.writeToStream(os, progress);
    return SUCCESS;
}

int FileUtil::unzipToDir(const std::string& zipfilePath, const std::string& destDirectory)
{
    if (!FileUtil::fileExists(zipfilePath)) {
        std::string msg = "FileUtil::unzipToDir(): ERROR: " + zipfilePath + " does not exits";
        errorMessage(msg);
        return FAILURE;
    }
    int result = createDirectoryConditionally(destDirectory);
    if (result != SUCCESS) {
        std::string msg = "FileUtil::unzipToDir(): failed to create directory " + destDirectory;
        errorMessage(msg);
        return result;
    }

    // Decompress the zip file
    File file(zipfilePath);
    File dir(destDirectory);
    ZipFile zipFile(file);

    Result zipResult = zipFile.uncompressTo(dir);

    if (zipResult.wasOk()) {
        std::string msg = "FileUtil::unzipToDir(): successfully unzipped " + zipfilePath + " to " + destDirectory;
        noteMessage(msg);
        return SUCCESS;
    }
    else {
        std::string msg = "FileUtil::unzipToDir(): ERROR: " + zipfilePath + " failed to unzip";
        errorMessage(msg);
        return FAILURE;
    }
}

std::string FileUtil::getRelativePathFromAbsolute(const std::string& filepath, const std::string& parentDir)
{
    if (!isAbsolutePath(filepath) || !isAbsolutePath(parentDir)) { return std::string(); } // return empty string
    File filepathFile = File(filepath);
    File parentDirFile = File(parentDir);
    String relativePath = filepathFile.getRelativePathFrom(parentDirFile);
    return relativePath.toStdString();
}

std::string FileUtil::getValidPathFromRelative(const std::string& directory, const std::string& relativePath)
{
    std::string filename = directory + fileSeparator() + relativePath;
    filename = setPathSeparators(filename);
    if (fileExists(filename)) { return filename; }
    else { return std::string(); }
}

std::string FileUtil::setPathSeparators(const std::string& filepath)
{
    std::string newFilepath = filepath;
#if defined(WINDOWS)
    std::replace( newFilepath.begin(), newFilepath.end(), '/', '\\'); // replace all slash to backslash
#elif defined(LINUX) || defined(MACOS)
    std::replace( newFilepath.begin(), newFilepath.end(), '\\', '/'); // replace all backslash to slash
#else
#error Unsupported OS in FileUtil::setPathSeparators()
#endif
    return newFilepath;
}

}
