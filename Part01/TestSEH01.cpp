// TestSEH01.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// For this sample to work, the following setting must be applied :
//
// SAFESEH Setting.
// 1. Linker | Advanced Properties : set Image Has Safe Exception Handlers to : No (/SAFESEH:NO)
// 2. Or Linker | Command Line : add the following option into the Additional Options box : /SAFESEH:NO
//
// _CRT_SECURE_NO_WARNINGS Setting.
// 1. C/C++ Preprocessor.
// 2. Preprocessor Definitions.
// 3. Add _CRT_SECURE_NO_WARNINGS.
//
// Also check out the following sites :
// Exception Handler not called in C
// https://stackoverflow.com/questions/19722308/exception-handler-not-called-in-c
// 
// Custom SEH handler with /SAFESEH
// https://stackoverflow.com/questions/12019689/custom-seh-handler-with-safeseh
// 
// /SAFESEH (Image has Safe Exception Handlers)
// https://docs.microsoft.com/en-us/cpp/build/reference/safeseh-image-has-safe-exception-handlers?view=msvc-170
//
// What SAFESEH:NO option actually do
// https://stackoverflow.com/questions/25081033/what-safesehno-option-actually-do
// 
// CppCon 2018: James McNellis “Unwinding the Stack: Exploring How C++ Exceptions Work on Windows”
// https://www.youtube.com/watch?v=COEv2kq_Ht8
//
// How to understand _Function_class_(name) in C++
// https://stackoverflow.com/questions/23502279/how-to-understand-function-class-name-in-c
//
// Annotating function parameters and return values
// https://docs.microsoft.com/en-us/cpp/code-quality/annotating-function-parameters-and-return-values?view=msvc-170
// 
// Annotating function behavior
// https://docs.microsoft.com/en-us/cpp/code-quality/annotating-function-behavior?view=msvc-170
//
// In particular, note the intended meaning of _Function_class_(name) :
// The name parameter is an arbitrary string that is designated by the user. 
// It exists in a namespace that is distinct from other namespaces. 
// A function, function pointer, or—most usefully—a function pointer type 
// may be designated as belonging to one or more function classes.
//



#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <winnt.h>

#define EXCEPTION_BUFFER_OVERFLOW       0xE0000001
#define EXCEPTION_TEST                  0xE0000002

#define DISPLAY_EXCEPTION_INFO(pExceptionRecord) \
    printf("An excepton occured at address : [0x%p]. Exception Code : [0x%08X]. Exception Flags : [0x%08X]\r\n", \
        pExceptionRecord->ExceptionAddress, \
        pExceptionRecord->ExceptionCode, \
        pExceptionRecord->ExceptionFlags); \
\
        if (pExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE) /* 1 Noncontinuable exception */\
            printf(" EXCEPTION_NONCONTINUABLE\r\n"); \
\
        if (pExceptionRecord->ExceptionFlags & EXCEPTION_UNWINDING) /* 2 Unwind is in progress */ \
            printf(" EXCEPTION_UNWINDING\r\n"); \
\
        if (pExceptionRecord->ExceptionFlags & EXCEPTION_EXIT_UNWIND) /* 4 Exit unwind is in progress */ \
            printf(" EXCEPTION_EXIT_UNWIND\r\n"); \
\
        if (pExceptionRecord->ExceptionFlags & EXCEPTION_STACK_INVALID) /* 8 Stack out of limits or unaligned */ \
            printf(" EXCEPTION_STACK_INVALID\r\n"); \
\
        if (pExceptionRecord->ExceptionFlags & EXCEPTION_NESTED_CALL) /* 0x10 Nested exception handler call */ \
            printf(" EXCEPTION_NESTED_CALL\r\n"); \
\
        if (pExceptionRecord->ExceptionFlags & EXCEPTION_TARGET_UNWIND) /* 0x20 Target unwind in progress */ \
            printf(" EXCEPTION_TARGET_UNWIND\r\n"); \
\
        if (pExceptionRecord->ExceptionFlags & EXCEPTION_COLLIDED_UNWIND) /* 0x40 Collided exception handler call */ \
            printf(" EXCEPTION_COLLIDED_UNWIND\r\n");





int g_iDividend = 1000;
int g_iDivisor = 0;

EXCEPTION_DISPOSITION NTAPI _Function_class_(EXCEPTION_ROUTINE) MyDivisionByZero01ExceptionRoutine
(
    _Inout_ struct _EXCEPTION_RECORD* pExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT* pContextRecord,
    _In_ PVOID DispatcherContext
)
{
    DISPLAY_EXCEPTION_INFO(pExceptionRecord)

    g_iDivisor = 1;

    return ExceptionContinueExecution;
}





int TestDivisionByZero01SEH()
{
    NT_TIB* TIB = (NT_TIB*)NtCurrentTeb();

    EXCEPTION_REGISTRATION_RECORD Registration;
    Registration.Handler = (PEXCEPTION_ROUTINE)(&MyDivisionByZero01ExceptionRoutine);
    Registration.Next = TIB->ExceptionList;
    TIB->ExceptionList = &Registration;

    int iValue = g_iDividend / g_iDivisor;

    TIB->ExceptionList = TIB->ExceptionList->Next;

    return iValue;
}





EXCEPTION_DISPOSITION NTAPI _Function_class_(EXCEPTION_ROUTINE) MyDivisionByZero02ExceptionRoutine
(
    _Inout_ struct _EXCEPTION_RECORD* pExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT* pContextRecord,
    _In_ PVOID DispatcherContext
)
{
    DISPLAY_EXCEPTION_INFO(pExceptionRecord)

    int* pDividendParam = (int*)((pContextRecord->Ebp) + 8);
    int* pDivisorParam = (int*)((pContextRecord->Ebp) + 12);

    printf("Dividend : [%d]. Divisor : [%d]\r\n",
        *pDividendParam,
        *pDivisorParam);

    *pDivisorParam = 1;

    return ExceptionContinueExecution;
}





int TestDivisionByZero02SEH(int iDividend, int iDivisor)
{
    NT_TIB* TIB = (NT_TIB*)NtCurrentTeb();

    EXCEPTION_REGISTRATION_RECORD Registration;
    Registration.Handler = (PEXCEPTION_ROUTINE)(&MyDivisionByZero02ExceptionRoutine);
    Registration.Next = TIB->ExceptionList;
    TIB->ExceptionList = &Registration;

    int iValue = iDividend / iDivisor;

    TIB->ExceptionList = TIB->ExceptionList->Next;

    return iValue;
}





EXCEPTION_DISPOSITION NTAPI _Function_class_(EXCEPTION_ROUTINE) MyBufferOverflowExceptionRoutine
(
    _Inout_ struct _EXCEPTION_RECORD* pExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT* pContextRecord,
    _In_ PVOID DispatcherContext
)
{
    DISPLAY_EXCEPTION_INFO(pExceptionRecord)

    void* pBufferAddress = (void*)(pExceptionRecord->ExceptionInformation[0]);
    size_t  stSize = pExceptionRecord->ExceptionInformation[1];

    printf("Overflowed Buffer : [0x%p]. Size : [%d]\r\n",
        pBufferAddress,
        stSize);

    char* pBuffer = new char[stSize + 1];
    memset(pBuffer, 0, stSize + 1);
    memcpy(pBuffer, pBufferAddress, stSize);

    printf("Overflowed Buffer Contents : [%s]\r\n", pBuffer);

    free(pBuffer);
    pBuffer = NULL;

    printf("iStackGuard Value : [0x%08X].\r\n",
        pExceptionRecord->ExceptionInformation[2]);

    int* pReturnAddress = (int*)(pExceptionRecord->ExceptionInformation[3]);

    printf("Return Address    : [0x%08X].\r\n", *pReturnAddress);

    return ExceptionContinueSearch;
}





void TestBufferOverflowSEH(const char* pString)
{
    int  iStackGuard = 0;
    char szCopy[8];

    NT_TIB* TIB = (NT_TIB*)NtCurrentTeb();
    EXCEPTION_REGISTRATION_RECORD Registration;
    Registration.Handler = (PEXCEPTION_ROUTINE)(&MyBufferOverflowExceptionRoutine);
    Registration.Next = TIB->ExceptionList;
    TIB->ExceptionList = &Registration;

    strcpy(szCopy, pString);

    if (iStackGuard != 0)
    {
        ULONG_PTR args[4] = { 0 };
        args[0] = (ULONG_PTR)(szCopy);
        args[1] = 8;
        args[2] = iStackGuard;
        args[3] = (ULONG_PTR)(&iStackGuard + 8);

        RaiseException(EXCEPTION_BUFFER_OVERFLOW, EXCEPTION_NONCONTINUABLE, 4, args);
    }

    TIB->ExceptionList = TIB->ExceptionList->Next;
}





EXCEPTION_DISPOSITION NTAPI _Function_class_(EXCEPTION_ROUTINE) MyTestExceptionRoutine
(
    _Inout_ struct _EXCEPTION_RECORD* pExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT* pContextRecord,
    _In_ PVOID DispatcherContext
)
{
    DISPLAY_EXCEPTION_INFO(pExceptionRecord)

    return ExceptionContinueSearch;
}





// Note that if pRegistration points to a Global structure,
// The exception handler will not be invoked.
int TestSEHGlobalExRegRec(EXCEPTION_REGISTRATION_RECORD* pRegistration)
{
    NT_TIB* TIB = (NT_TIB*)NtCurrentTeb();

    pRegistration->Handler = (PEXCEPTION_ROUTINE)(&MyTestExceptionRoutine);
    pRegistration->Next = TIB->ExceptionList;
    TIB->ExceptionList = pRegistration;

    RaiseException(EXCEPTION_TEST, EXCEPTION_NONCONTINUABLE, 0, NULL);

    TIB->ExceptionList = TIB->ExceptionList->Next;

    return 0;
}





// For testing with TestSEHGlobalExRegRec().
// If this global Registration is used, TestSEHGlobalExRegRec() will not work.
EXCEPTION_REGISTRATION_RECORD Registration;

int main()
{
    int iValue = TestDivisionByZero01SEH();

    iValue = TestDivisionByZero02SEH(1000, 0);

    TestBufferOverflowSEH((const char*)"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");    

    // Uncomment the declaration below to make TestSEHGlobalExRegRec() work.
    //EXCEPTION_REGISTRATION_RECORD Registration;
    TestSEHGlobalExRegRec(&Registration);
}





// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
