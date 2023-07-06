
#include <stdio.h>
#include <windows.h>

const int Num_Threads = 5;
DWORD WINAPI ChildThreadFunc(LPVOID lpMutex);

int main()
{
    int i = 0;
    DWORD dwNewThreadID;
    HANDLE hChildThreads[Num_Threads];

    //create mutext with this thread the owner of the mutex
    HANDLE hMutex = CreateMutex(NULL, TRUE, NULL);

    while (i < Num_Threads)
    {
        hChildThreads[i] = CreateThread(NULL, NULL, &ChildThreadFunc, (void *)hMutex, 0, &dwNewThreadID);
        printf("In parent thread, child thread ID %p created.\n", dwNewThreadID);
        i++;
    }

    printf("In parent thread, releasing mutex.\n");
    ReleaseMutex(hMutex);

    Sleep(6000);

    return 0;
}

DWORD WINAPI ChildThreadFunc(LPVOID lpMutex)
{
    //convert parameter to mutex handle
    HANDLE hMutex = (HANDLE)lpMutex;

    //save the thread ID
    DWORD dwThreadID = GetCurrentThreadId();
    DWORD dwResult = WaitForSingleObject(hMutex, INFINITE);
    if (dwResult == WAIT_OBJECT_0)
    {
        printf("Thread %p acquired mutex.\n", dwThreadID);

        //hold the mutex
        Sleep(1000);
        printf("Thread %p releasing the mutex.\n", dwThreadID);
        ReleaseMutex(hMutex);
    }
    else
    {
        return 1;
    }
    return 0;
}
