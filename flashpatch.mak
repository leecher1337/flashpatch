# Microsoft Developer Studio Generated NMAKE File, Based on flashpatch.dsp
!IF "$(CFG)" == ""
CFG=flashpatch - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. flashpatch - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "flashpatch - Win32 Release" && "$(CFG)" != "flashpatch - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "flashpatch.mak" CFG="flashpatch - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "flashpatch - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "flashpatch - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "flashpatch - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\flashpatch.exe"


CLEAN :
	-@erase "$(INTDIR)\addacl.obj"
	-@erase "$(INTDIR)\flashpatch.obj"
	-@erase "$(INTDIR)\res.res"
	-@erase "$(INTDIR)\takeown.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\flashpatch.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /GS- /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\flashpatch.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
RSC_PROJ=/l 0xc07 /fo"$(INTDIR)\res.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\flashpatch.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\flashpatch.pdb" /out:"$(OUTDIR)\flashpatch.exe" 
LINK32_OBJS= \
	"$(INTDIR)\addacl.obj" \
	"$(INTDIR)\flashpatch.obj" \
	"$(INTDIR)\takeown.obj" \
	"$(INTDIR)\res.res"

"$(OUTDIR)\flashpatch.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "flashpatch - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\flashpatch.exe"


CLEAN :
	-@erase "$(INTDIR)\addacl.obj"
	-@erase "$(INTDIR)\flashpatch.obj"
	-@erase "$(INTDIR)\res.res"
	-@erase "$(INTDIR)\takeown.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\flashpatch.exe"
	-@erase "$(OUTDIR)\flashpatch.ilk"
	-@erase "$(OUTDIR)\flashpatch.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /GS- /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\flashpatch.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
RSC_PROJ=/l 0xc07 /fo"$(INTDIR)\res.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\flashpatch.bsc" 
BSC32_SBRS= \
	
LINK32=xilink6.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\flashpatch.pdb" /debug /out:"$(OUTDIR)\flashpatch.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\addacl.obj" \
	"$(INTDIR)\flashpatch.obj" \
	"$(INTDIR)\takeown.obj" \
	"$(INTDIR)\res.res"

"$(OUTDIR)\flashpatch.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("flashpatch.dep")
!INCLUDE "flashpatch.dep"
!ELSE 
!MESSAGE Warning: cannot find "flashpatch.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "flashpatch - Win32 Release" || "$(CFG)" == "flashpatch - Win32 Debug"
SOURCE=.\addacl.c

"$(INTDIR)\addacl.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flashpatch.c

"$(INTDIR)\flashpatch.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\takeown.c

"$(INTDIR)\takeown.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\res.rc

"$(INTDIR)\res.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

