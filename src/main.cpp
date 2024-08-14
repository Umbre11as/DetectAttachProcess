#include <ntifs.h>
#include <ntstrsafe.h>

#define Log(format, ...) DbgPrintEx(0, 0, format, __VA_ARGS__)

struct EPROCESS {
private:
    unsigned char _pad0[0x448];
public:
    LIST_ENTRY ActiveProcessLinks; // 0x448
private:
    unsigned char _pad1[0x150];
public:
    UCHAR ImageFileName[15]; // 0x5a8
private:
    unsigned char _pad2[0x29];
public:
    LIST_ENTRY ThreadListHead; // 0x5e0
    volatile ULONG ActiveThreads; // 0x5f0
};

struct KTHREAD {
private:
    unsigned char _pad0[0x98];
public:
    union {
        struct _KAPC_STATE ApcState; //0x98
        struct {
            UCHAR ApcStateFill[43]; //0x98
            CHAR Priority; //0xc3
            ULONG UserIdealProcessor; //0xc4
        };
    };
private:
    unsigned char _pad1[0x230];
public:
    LIST_ENTRY ThreadListEntry; // 0x2f8
private:
    unsigned char _pad2[0x128];
};

struct ETHREAD {
public:
    KTHREAD Tcb; // 0x0
private:
    unsigned char _pad0[0xB8];
public:
    LIST_ENTRY ThreadListEntry; // 0x4e8
};

extern "C" {
    PCSTR PsGetProcessImageFileName(IN EPROCESS* Process); // Undocumented function. ImageFileName курит в сторонке :)
}

[[noreturn]] void NTAPI Thread(IN PVOID) {
    while (true) {
        auto* systemProcess = reinterpret_cast<EPROCESS*>(PsInitialSystemProcess);
        EPROCESS* process = systemProcess;

        do {
            PCSTR processName = PsGetProcessImageFileName(process);

            if (process != systemProcess && process->ActiveThreads) {
                PLIST_ENTRY currentThreadEntry = &process->ThreadListHead;
                PLIST_ENTRY nextThreadEntry = process->ThreadListHead.Flink;

                while (currentThreadEntry != nextThreadEntry) {
                    ETHREAD* thread = CONTAINING_RECORD(nextThreadEntry, ETHREAD, ThreadListEntry);
                    auto* attached = reinterpret_cast<EPROCESS*>(thread->Tcb.ApcState.Process);
                    PCSTR attachedProcessName = PsGetProcessImageFileName(attached);
                    if (attached != process && strcmp(attachedProcessName, processName) != 0) {
                        Log("%s attached to %s\n", attachedProcessName, processName);
                        break;
                    }

                    nextThreadEntry = nextThreadEntry->Flink;
                }
            }

            process = CONTAINING_RECORD(process->ActiveProcessLinks.Flink, EPROCESS, ActiveProcessLinks);
        } while (process != systemProcess);
    }
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT, IN PUNICODE_STRING) {
    HANDLE handle;
    if (NT_SUCCESS(PsCreateSystemThread(&handle, GENERIC_ALL, nullptr, nullptr, nullptr, Thread, nullptr))) {
        Log("Thread started -> %p\n", handle);
        ZwClose(handle);
    }

    return STATUS_SUCCESS;
}
