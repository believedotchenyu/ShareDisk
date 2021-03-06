#ifndef _FILE_CONTROL_H_
#define _FILE_CONTROL_H_

#include <memory>
#include <string>
#include <vector>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <map>

#include "common.h"
#include "networking.h"
#include "protocol.h"

using namespace std;

struct KeyEntry
{
    string name;
    string keystring;
    SecretKey key;
};

struct File
{
    char filename[FILENAME_MAX_SIZE];
    time_t timestamp;
    bool is_deleted;
    uint32_t extra_length;
    uint8_t extra_data[16];
};

class FileControl
{
public:
    FileControl(string pd_path, vector<string> keystrings);
    ~FileControl();

    void Init();

    string Resolve(const string& path) const;
    string Pathname(const string& path) const;
    bool IsAccessible(const char *path) const; // path是否可访问
    bool IsTopLevel(const char *path) const; // path是否是根目录或者一级目录
    vector<string> KeyNames() const;

    File* FindFile(const char *path);

    void Sync(const char *path); // 如果有更新，将缓存同步到磁盘上
    void SyncDir(const char *path); // 如果有更新，将缓存同步到磁盘上
    void ClearCache(const char *path); // 删除缓存

    int NewFile(const char *path, int flags, mode_t mode);
    int ReadFile(const char *path, int fd, char *buf, size_t size, off_t offset);
    int WriteFile(const char *path, int fd, const char *buf, size_t size, off_t offset);
    int DeleteFile(const char *path);
    int RenameFile(const char *from, const char *to);

private:
    string PathJoin(string A, string B) const;
    string FirstPath(const string& path) const;
    size_t FileSize(const string& filepath) const;
    void LoadCFG();
    void SaveCFG();

    data_t Touch(const char *path, bool readonly = true);

    data_t LoadFile(const char* path); // 从磁盘上载入并解码，不管理缓存
    int SaveFile(const char* path, int fd, data_t decoded_data); // 写入磁盘文件并加密，不管理缓存

    void StartThread();
    void BroadcastFile(const char* path);

private:
    string pd_path;
    string cfg_filename;
    vector<File> files;
    vector<KeyEntry> keys;
    Networking* net;

    thread recv_thread, sync_thread;
    mutex sync_mutex;
    map<string, time_t> last_hit;
    map<string, time_t> last_modify;
    map<string, data_t> file_cache;
    map<string, bool> is_dirty;
};

#endif // _FILE_CONTROL_H_
