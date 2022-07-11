namespace tom {

function void free_file_memory(void* memory)
{
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    } else {
        INVALID_CODE_PATH;
    }
}

function bool write_entire_file(const char* file_path, u64 buf_size, void* buf)
{
    b32 success = false;

    HANDLE file_handle = CreateFileA(file_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if (WriteFile(file_handle, memory, (DWORD)memory_size, &bytes_written, 0)) {
            success = (bytes_written == memory_size);
        } else {
            printf("ERROR-> Failed to write file contents!\n");
        }
        CloseHandle(file_handle);
    } else {
        printf("ERROR-> Failed to oepn file handle!\n");
    }

    return success;
}


function file_read_result read_entire_file(const char* file_path)
{
    debug_read_file_result file = {};

    HANDLE file_handle { CreateFileA(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL, 0) };
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(file_handle, &fileSize)) {
            u32 fileSize32 = safe_truncate_u32_to_u64(fileSize.QuadPart);
            file.contents  = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (file.contents) {
                DWORD bytesRead;
                if (ReadFile(file_handle, file.contents, (DWORD)fileSize.QuadPart, &bytesRead, 0) &&
                    fileSize32 == bytesRead) {
                    file.content_size = fileSize32;
                } else {
                    _debug_platform_free_file_memory(thread, file.contents);
                    file.contents = 0;
                }
            } else {
                printf("ERROR-> Failed to read file contents!\n");
            }
        } else {
            printf("ERROR-> Failed to open file handle!\n");
        }
        CloseHandle(file_handle);
    }
    return file;
}

}