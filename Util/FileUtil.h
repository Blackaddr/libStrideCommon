/*
 * FileUtil.h
 *
 *  Created on: Jan. 31, 2021
 *      Author: blackaddr
 */

#ifndef UTIL_FILEUTIL_H_
#define UTIL_FILEUTIL_H_

#include <string>

namespace stride {

class FileUtil {
public:

    static std::string getSystemTempDirectory();

    static std::string fileSeparator();

    static int writeStringToFile(const std::string& str, const std::string& filename, bool appendToFile=false);
    static int writeStringToFile(const void* dataPtr, size_t numBytes, const std::string& filename, bool appendToFile=false);

    static int readFileToString(const std::string& filename, std::string& strOutput);

    static bool directoryExists(const std::string& directory);
    static int  createDirectory(const std::string& directory);
    static int  createDirectoryConditionally(const std::string& directory);
    static int  deleteDirectory(const std::string& directory);
    static int  deleteDirectoryIfExists(const std::string& directory);
    static int  copyDirectoryFromTo(const std::string& srcDir, const std::string& destDir);
    static int  copyDirectoryFromInTo(const std::string& srcDir, const std::string& destDir);
    static std::string getParentDir(const std::string filepath);

    // get a relative path by removing the specified parent path
    static std::string getChildDirs(const std::string& filepath, const std::string& parentDir);

    static int copyFile(const std::string& destFilename, const std::string& srcFilename);
    static int copyFileToDir(const std::string& destDir, const std::string& srcFilename);

    /// This function will copy a src file to a destination dir but will create subdirs. The specified
    /// root directory is removed from the src file path, then append to the destination root directory
    /// @param destRootDir the target root directory
    /// @param srcRootDir the root directory
    /// @param srcFilename the full path to the source file
    /// @returns 0 on SUCCESS, else non-zero FAILURE
    static int copyFileWithSubDirsToDir(const std::string& destRootDir, const std::string srcRootDir, const std::string& srcFilename);

    static int createParentDirectoryTree(const std::string& directoryPath);

    static int deleteFile(const std::string& filename);
    static int deleteFilePattern(const std::string& directory, const std::string& filePattern);
    static int deleteFileIfExists(const std::string& filename);

    static bool fileExists(const std::string& filename);
    static size_t getFileSize(const std::string& filename);

    static bool isAbsolutePath(const std::string& filenamePath);

    // converts a relative path from current working dir to absolute path
    static std::string convertIfRelativePath(const std::string& filenamePath);

    static int createZipfileFromDir(const std::string& zipfilePath, const std::string& srcDirectory);
    static int unzipToDir(const std::string& zipfilePath, const std::string& destDirectory);

    static std::string getFilenameFromPath(const std::string& filenamePath);
    static std::string getFilenameFromRelativePath(const std::string& filenamePath);
    static std::string getFilenameExtension(const std::string& filenamePath);
    static std::string getFilenameWithoutExtension(const std::string& filenamePath);
    static std::string getFolderFromPath(const std::string& filepath);

    static std::string getRelativePathFromAbsolute(const std::string& filepath, const std::string& parentDir);

    /// Combines the directory with the relative path, then check if the file exists. If so, returns that absolute path
    static std::string getValidPathFromRelative(const std::string& directory, const std::string& relativePath);

    static std::string setPathSeparators(const std::string& filepath);
};

}

#endif /* SOURCE_UTIL_FILEUTIL_H_ */
