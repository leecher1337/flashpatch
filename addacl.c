#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "addacl.h"

void ShowModuleError(LPSTR pszModError, LPSTR pszError);

typedef BOOL (WINAPI *SetSecurityDescriptorControlFnPtr)(
   IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
   IN SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
   IN SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet);

typedef struct {
    PSECURITY_DESCRIPTOR pFileSD;
    PACL                 pNewACL;
} TYP_INST;

static void FreeInst(TYP_INST *pInst);
static BOOL ShowError(TYP_INST *pInst, LPSTR pszError);

BOOL AddAccessRights(LPSTR lpszFileName, LPVOID pUserSID, 
      DWORD dwAccessMask) 
{
    TYP_INST Inst = {0};

   // File SD variables.  
   DWORD          cbFileSD       = 0;

   // New SD variables.
   SECURITY_DESCRIPTOR  newSD;

   // ACL variables.
   PACL           pACL           = NULL;
   BOOL           fDaclPresent;
   BOOL           fDaclDefaulted;
   ACL_SIZE_INFORMATION AclInfo;

   // New ACL variables.
   DWORD          cbNewACL       = 0;

   // Temporary ACE.
   LPVOID         pTempAce       = NULL;
   UINT           CurrentAceIndex = 0;

   UINT           newAceIndex = 0;

   // Assume function will fail.
   BOOL           fResult        = FALSE;
   BOOL           fAPISuccess;

   SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

   // New APIs available only in Windows 2000 and above for setting 
   // SD control
   SetSecurityDescriptorControlFnPtr _SetSecurityDescriptorControl = NULL;

  // 
  // STEP 2: Get security descriptor (SD) of the file specified.
  // 
  if (fAPISuccess = GetFileSecurity(lpszFileName, 
        secInfo, Inst.pFileSD, 0, &cbFileSD))
        return ShowError (&Inst, "GetFileSecurity() failed.");
  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
      Inst.pFileSD = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbFileSD);
      if (!Inst.pFileSD) return ShowError (&Inst, "Out of memory");
      fAPISuccess = GetFileSecurity(lpszFileName, 
            secInfo, Inst.pFileSD, cbFileSD, &cbFileSD);
  }
  if (!fAPISuccess) return ShowError (&Inst, "GetFileSecurity() failed.");

  // 
  // STEP 3: Initialize new SD.
  // 
  if (!InitializeSecurityDescriptor(&newSD, SECURITY_DESCRIPTOR_REVISION)) 
        return ShowError (&Inst, "InitializeSecurityDescriptor() failed.");

  // 
  // STEP 4: Get DACL from the old SD.
  // 
  if (!GetSecurityDescriptorDacl(Inst.pFileSD, &fDaclPresent, &pACL,
        &fDaclDefaulted)) return ShowError (&Inst, "GetSecurityDescriptorDacl() failed.");

  // 
  // STEP 5: Get size information for DACL.
  // 
  AclInfo.AceCount = 0; // Assume NULL DACL.
  AclInfo.AclBytesFree = 0;
  AclInfo.AclBytesInUse = sizeof(ACL);

  if (pACL == NULL)
     fDaclPresent = FALSE;

  // If not NULL DACL, gather size information from DACL.
  if (fDaclPresent) {        
     if (!GetAclInformation(pACL, &AclInfo, 
           sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
           return ShowError (&Inst, "GetAclInformation() failed.");
  }

  // 
  // STEP 6: Compute size needed for the new ACL.
  // 
  cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) 
        + GetLengthSid(pUserSID) - sizeof(DWORD);

  // 
  // STEP 7: Allocate memory for new ACL.
  // 
  if (!(Inst.pNewACL = (PACL) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbNewACL)))
      return ShowError (&Inst, "Out of memory.");

  // 
  // STEP 8: Initialize the new ACL.
  // 
  if (!InitializeAcl(Inst.pNewACL, cbNewACL, ACL_REVISION2))
      return ShowError (&Inst, "InitializeAcl() failed");

  // 
  // STEP 9 If DACL is present, copy all the ACEs from the old DACL
  // to the new DACL.
  // 
  // The following code assumes that the old DACL is
  // already in Windows 2000 preferred order.  To conform 
  // to the new Windows 2000 preferred order, first we will 
  // copy all non-inherited ACEs from the old DACL to the 
  // new DACL, irrespective of the ACE type.
  // 
  
  newAceIndex = 0;

  if (fDaclPresent && AclInfo.AceCount) {

     for (CurrentAceIndex = 0; 
           CurrentAceIndex < AclInfo.AceCount;
           CurrentAceIndex++) {

        // 
        // STEP 10: Get an ACE.
        // 
        if (!GetAce(pACL, CurrentAceIndex, &pTempAce))
            return ShowError (&Inst, "GetAce() failed.");

        // 
        // STEP 11: Check if it is a non-inherited ACE.
        // If it is an inherited ACE, break from the loop so
        // that the new access allowed non-inherited ACE can
        // be added in the correct position, immediately after
        // all non-inherited ACEs.
        // 
        if (((ACCESS_ALLOWED_ACE *)pTempAce)->Header.AceFlags
           & INHERITED_ACE)
           break;

        // 
        // STEP 12: Skip adding the ACE, if the SID matches
        // with the account specified, as we are going to 
        // add an access allowed ACE with a different access 
        // mask.
        // 
        if (EqualSid(pUserSID,
           &(((ACCESS_ALLOWED_ACE *)pTempAce)->SidStart)))
           continue;

        // 
        // STEP 13: Add the ACE to the new ACL.
        // 
        if (!AddAce(Inst.pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
              ((PACE_HEADER) pTempAce)->AceSize)) 
              return ShowError (&Inst, "AddAce() failed.");

        newAceIndex++;
     }
  }

  // 
  // STEP 14: Add the access-allowed ACE to the new DACL.
  // The new ACE added here will be in the correct position,
  // immediately after all existing non-inherited ACEs.
  // 
  if (!AddAccessAllowedAce(Inst.pNewACL, ACL_REVISION2, dwAccessMask,
        pUserSID)) 
        return ShowError (&Inst, "AddAccessAllowedAce() failed.");

  // 
  // STEP 15: To conform to the new Windows 2000 preferred order,
  // we will now copy the rest of inherited ACEs from the
  // old DACL to the new DACL.
  // 
  if (fDaclPresent && AclInfo.AceCount) {

     for (; 
          CurrentAceIndex < AclInfo.AceCount;
          CurrentAceIndex++) {

        // 
        // STEP 16: Get an ACE.
        // 
        if (!GetAce(pACL, CurrentAceIndex, &pTempAce))
            return ShowError (&Inst, "GetAce() failed.");

        // 
        // STEP 17: Add the ACE to the new ACL.
        // 
        if (!AddAce(Inst.pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
              ((PACE_HEADER) pTempAce)->AceSize))
              return ShowError (&Inst, "AddAce() failed.");
     }
  }

  // 
  // STEP 18: Set the new DACL to the new SD.
  // 
  if (!SetSecurityDescriptorDacl(&newSD, TRUE, Inst.pNewACL, 
        FALSE)) return ShowError (&Inst, "SetSecurityDescriptorDacl() failed.");

  // 
  // STEP 19: Copy the old security descriptor control flags 
  // regarding DACL automatic inheritance for Windows 2000 or 
  // later where SetSecurityDescriptorControl() API is available
  // in advapi32.dll.
  // 
  _SetSecurityDescriptorControl = (SetSecurityDescriptorControlFnPtr)
        GetProcAddress(GetModuleHandle("advapi32.dll"),
        "SetSecurityDescriptorControl");
  if (_SetSecurityDescriptorControl) {

     SECURITY_DESCRIPTOR_CONTROL controlBitsOfInterest = 0;
     SECURITY_DESCRIPTOR_CONTROL controlBitsToSet = 0;
     SECURITY_DESCRIPTOR_CONTROL oldControlBits = 0;
     DWORD dwRevision = 0;

     if (!GetSecurityDescriptorControl(Inst.pFileSD, &oldControlBits,
        &dwRevision)) return ShowError (&Inst, "GetSecurityDescriptorControl() failed.");

     if (oldControlBits & SE_DACL_AUTO_INHERITED) {
        controlBitsOfInterest =
           SE_DACL_AUTO_INHERIT_REQ |
           SE_DACL_AUTO_INHERITED;
        controlBitsToSet = controlBitsOfInterest;
     }
     else if (oldControlBits & SE_DACL_PROTECTED) {
        controlBitsOfInterest = SE_DACL_PROTECTED;
        controlBitsToSet = controlBitsOfInterest;
     }
     
     if (controlBitsOfInterest) {
        if (!_SetSecurityDescriptorControl(&newSD,
           controlBitsOfInterest,
           controlBitsToSet)) 
           return ShowError (&Inst, "SetSecurityDescriptorControl() failed.");
     }
  }

  // 
  // STEP 20: Set the new SD to the File.
  // 
  if (!SetFileSecurity(lpszFileName, secInfo,
        &newSD)) return ShowError (&Inst, "SetFileSecurity() failed.");

  fResult = TRUE;
  FreeInst(&Inst);
   
  return fResult;
}

// =============================================================
// Private
// =============================================================
static void FreeInst(TYP_INST *pInst)
{
  if (pInst->pFileSD)
     HeapFree(GetProcessHeap(), 0, pInst->pFileSD);

  if (pInst->pNewACL)
     HeapFree (GetProcessHeap(), 0, pInst->pNewACL);
}

// --------------------------------------------------------------

static BOOL ShowError(TYP_INST *pInst, LPSTR pszError)
{
	ShowModuleError("Failed setting up ACL", pszError);
    FreeInst(pInst);
    return FALSE;
}
