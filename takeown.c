#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "addacl.h"

BOOL ShowModuleError(LPSTR pszModError, LPSTR pszError);

static BOOL
AssertTakeOwnership(
    HANDLE TokenHandle
    );
static BOOL
GetTokenHandle(
    PHANDLE TokenHandle
    );
static BOOL
VariableInitialization();
static BOOL ShowError(LPSTR pszError);

PSID AliasAdminsSid = NULL;
PSID SeWorldSid;
static SID_IDENTIFIER_AUTHORITY    SepNtAuthority = SECURITY_NT_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY    SepWorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;

BOOL TakeOwnership(LPSTR lpFileName)
{
    BOOL Result;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    HANDLE TokenHandle;

    Result = VariableInitialization();
    if ( !Result ) return ShowError ("SID Initialization failed");

    Result = GetTokenHandle( &TokenHandle );
    if ( !Result ) return ShowError ("Unable to obtain the handle to our token, exiting\n");
 
     //
    // Attempt to put a NULL Dacl on the object
    //
    InitializeSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

    Result = SetSecurityDescriptorDacl (
                 &SecurityDescriptor,
                 TRUE,
                 NULL,
                 FALSE
                 );
 
    if ( !Result ) return ShowError ("SetSecurityDescriptorDacl failed");

    Result = SetFileSecurity(
                 lpFileName,
                 DACL_SECURITY_INFORMATION,
                 &SecurityDescriptor
                 );
    if ( Result ) return TRUE;

    //
    // That didn't work.
    //

    //
    // Attempt to make Administrator the owner of the file.
    //

    Result = SetSecurityDescriptorOwner (
                 &SecurityDescriptor,
                 AliasAdminsSid,
                 FALSE
                 );
    if ( !Result ) return ShowError ("SetSecurityDescriptorOwner failed");

    Result = SetFileSecurity(
                 lpFileName,
                 OWNER_SECURITY_INFORMATION,
                 &SecurityDescriptor
                 );
    if ( !Result ) {

        //
        // That didn't work either.
        //
 
        //
        // Assert TakeOwnership privilege, then try again.  Note that
        // since the privilege is only enabled for the duration of 
        // this process, we don't have to worry about turning it off
        // again.
        //
        Result = AssertTakeOwnership( TokenHandle );
        if ( !Result ) return ShowError("Could not enable SeTakeOwnership privilege");

        Result = SetFileSecurity(
                     lpFileName,
                     OWNER_SECURITY_INFORMATION,
                     &SecurityDescriptor
                     );
        if ( !Result ) return ShowError ("Unable to assign Administrator as owner");
    }
    //
    // Try to put a benign DACL onto the file again
    //
    Result = SetFileSecurity(
                 lpFileName,
                 DACL_SECURITY_INFORMATION,
                 &SecurityDescriptor
                 );
    if ( !Result ) return ShowError ("SetFileSecurity unexpectedly failed");

    return TRUE;
}

// --------------------------------------------------------------

BOOL GrantAllPrivileges (LPSTR lpszFileName)
{
    VariableInitialization();
    return AddAccessRights (lpszFileName, SeWorldSid, GENERIC_ALL);
}

// =============================================================
// Private
// =============================================================
static BOOL
GetTokenHandle(
    PHANDLE TokenHandle
    )
//
// This routine will open the current process and return
// a handle to its token.
//
// These handles will be closed for us when the process
// exits.
//
{
    HANDLE ProcessHandle;
    BOOL Result;
    ProcessHandle = OpenProcess(
                        PROCESS_QUERY_INFORMATION,
                        FALSE,
                        GetCurrentProcessId()
                        );
    if ( ProcessHandle == NULL ) {
        //
        // This should not happen
        //
        return( FALSE );
    }

    Result = OpenProcessToken (
                 ProcessHandle,
                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                 TokenHandle
                 );
    if ( !Result ) {
        //
        // This should not happen
        //
        return( FALSE );
    }
    return( TRUE );
}

// --------------------------------------------------------------

static BOOL
AssertTakeOwnership(
    HANDLE TokenHandle
    )
//
// This routine turns on SeTakeOwnershipPrivilege in the current
// token.  Once that has been accomplished, we can open the file
// for WRITE_OWNER even if we are denied that access by the ACL
// on the file.
{
    LUID TakeOwnershipValue;
    BOOL Result;
    TOKEN_PRIVILEGES TokenPrivileges;

    //
    // First, find out the value of TakeOwnershipPrivilege
    //

    Result = LookupPrivilegeValue(
                 NULL,
                 "SeTakeOwnershipPrivilege",
                 &TakeOwnershipValue
                 );
    if ( !Result ) return ShowError ("Unable to obtain value of TakeOwnership privilege");

    //
    // Set up the privilege set we will need
    //
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = TakeOwnershipValue;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
 

    (VOID) AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof( TOKEN_PRIVILEGES ),
                NULL,
                NULL
                );
    if ( GetLastError() != NO_ERROR ) {
        return( FALSE );
    } else {
        return( TRUE );
    }
}

// --------------------------------------------------------------
 
static BOOL
VariableInitialization()
//
// Create some useful SIDs.
//
{
    static BOOL Result = FALSE;

    if (Result) return TRUE;
    Result = AllocateAndInitializeSid(
                 &SepNtAuthority,
                 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 &AliasAdminsSid
                 );
    if ( !Result ) {
        return( FALSE );
    }

    Result = AllocateAndInitializeSid(
                 &SepWorldSidAuthority,
                 1,
                 SECURITY_WORLD_RID,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 &SeWorldSid
                 );
    if ( !Result ) {
        return( FALSE );
    }
    return( TRUE );
}

// --------------------------------------------------------------

static BOOL ShowError(LPSTR pszError)
{
	return ShowModuleError("Failed taking ownership", pszError);
}
