// TestSEH01.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// For this sample to work, the following setting must be applied :
//
// SAFESEH Setting.
// 1. Linker Properties.
// 2. Command Line property page.
// 3. Enter the following option into the Additional Options box : /SAFESEH:NO
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


#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <winnt.h>

#define EXCEPTION_BUFFER_OVERFLOW       0xE0000001




extern "C" EXCEPTION_DISPOSITION NTAPI MyDivisionByZeroExceptionRoutine
(
    _Inout_ struct _EXCEPTION_RECORD* pExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT* pContextRecord,
    _In_ PVOID DispatcherContext
)
{
    printf("An excepton occured at address : [0x%p]. Exception Code : [0x%08X]\r\n",
        pExceptionRecord->ExceptionAddress,
        pExceptionRecord->ExceptionCode);

    int* pDividendParam = (int*)((pContextRecord->Ebp) + 8);
    int* pDivisorParam = (int*)((pContextRecord->Ebp) + 12);

    printf("Dividend : [%d]. Divisor : [%d]\r\n",
        *pDividendParam,
        *pDivisorParam);

    *pDivisorParam = 1;

    return ExceptionContinueExecution;
}





int TestDivisionByZeroSEH(int iDividend, int iDivisor)
{
    NT_TIB* TIB = (NT_TIB*)NtCurrentTeb();

    EXCEPTION_REGISTRATION_RECORD Registration;
    Registration.Handler = (PEXCEPTION_ROUTINE)(&MyDivisionByZeroExceptionRoutine);
    Registration.Next = TIB->ExceptionList;
    TIB->ExceptionList = &Registration;

    int iValue = iDividend / iDivisor;

    TIB->ExceptionList = TIB->ExceptionList->Next;

    return iValue;
}





extern "C" EXCEPTION_DISPOSITION NTAPI MyBufferOverflowExceptionRoutine
(
    _Inout_ struct _EXCEPTION_RECORD* pExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ struct _CONTEXT* pContextRecord,
    _In_ PVOID DispatcherContext
)
{
    printf("An excepton occured at address : [0x%p]. Exception Code : [0x%08X]\r\n",
        pExceptionRecord->ExceptionAddress,
        pExceptionRecord->ExceptionCode);

    void* pBufferAddress = (void*)(pExceptionRecord->ExceptionInformation[0]);
    size_t  stSize = pExceptionRecord->ExceptionInformation[1];

    printf("Overflowed Buffer : [0x%08X]. Size : [%d]\r\n",
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





int main()
{
    int iValue = TestDivisionByZeroSEH(1000, 0);

    printf("%d\r\n", iValue);

    TestBufferOverflowSEH((const char*)"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
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
