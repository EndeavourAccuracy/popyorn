/* popyorn v0.9b (September 2018)
 * Copyright (C) 2018 Norbert de Jonge <mail@norbertdejonge.nl>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see [ www.gnu.org/licenses/ ].
 *
 * To properly read this code, set your program's tab stop to: 2.
 */

/*========== Includes ==========*/
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h> /*** Maybe remove, and just change CHAR_BIT to 8? ***/
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
#include <windows.h>
#undef PlaySound
#endif

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_thread.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
/*========== Includes ==========*/

/*========== Defines ==========*/
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
#define SLASH "\\"
#define DEVNULL "NUL"
#define HERE ""
#define BATCH_FILE "playtest.bat"
#else
#define SLASH "/"
#define DEVNULL "/dev/null"
#define HERE "./"
#define BATCH_FILE "playtest.sh"
#endif

#define PNG_VARIOUS "png" SLASH "various" SLASH
#define PNG_BUTTONS "png" SLASH "buttons" SLASH
#define PNG_GAMEPAD "png" SLASH "gamepad" SLASH
#define PNG_DUNGEON "png" SLASH "dungeon" SLASH
#define PNG_PALACE "png" SLASH "palace" SLASH
#define PNG_SELECTED "png" SLASH "selected" SLASH
#define PNG_LIVING "png" SLASH "living" SLASH
#define PNG_SLIVING "png" SLASH "sliving" SLASH
#define PNG_EXTRAS "png" SLASH "extras" SLASH
#define PNG_ROOMS "png" SLASH "rooms" SLASH
#define PNG_MINI "png" SLASH "mini" SLASH

#define EXIT_NORMAL 0
#define EXIT_ERROR 1
#define EDITOR_NAME "popyorn"
#define EDITOR_VERSION "v0.9b (September 2018)"
#define COPYRIGHT "Copyright (C) 2018 Norbert de Jonge"
#define BIN_DIR "bin"
#define BACKUP BIN_DIR SLASH "backup.bak"
#define MAX_PATHFILE 200
#define MAX_OPTION 100
#define MAX_LEVEL 15
#define ROOMS 24
#define TILES 30
#define EVENTS 256
#define WINDOW_WIDTH 690
#define WINDOW_HEIGHT 459
#define MAX_IMG 200
#define MAX_CON 30
#define NUM_SOUNDS 20 /*** Sounds that may play at the same time. ***/
#define BAR_FULL 435
#define MAX_TEXT 100
#define REFRESH 25 /*** That is 40 frames per second, 1000/25. ***/
#define MAX_TOWRITE 720
#define ADJ_BASE_X 419
#define ADJ_BASE_Y 62
#define MAX_STATUS 100 /*** Cannot be more than MAX_TEXT. ***/
#define MAX_DATA 720

/*** Map window ***/
#define MAP_BIG_AREA_W 1225
#define MAP_BIG_AREA_H 745
#define MAX_ZOOM 7
#define DEFAULT_ZOOM 4

/*** B9 3E 60 1A 3B 7C [00 3C] BA 8C 3B 7C 02 CF ***/
#define OFFSET_STARTMIN 0x9B3C
/*** CF BA 80 3B 7C [00 03] BA 46 42 6D 96 36 ***/
#define OFFSET_STARTHP 0x9B48
/*** 64 B2 6D BA 62 54 8F [66] 0A 3B 7C 00 0F BA 64 4E ***/
#define OFFSET_SKIPPOT 0x4C5A
/*** BA 22 60 0E 0C 2D [00 05] B9 73 66 06 3B 7C ***/
#define OFFSET_WINROOM 0x8B4C
/*** 4E BA 02 C2 3B 7C [00 01] BA 64 42 2D A1 24 ***/
#define OFFSET_STARTLVL 0x3EDE

#ifndef O_BINARY
#define O_BINARY 0
#endif
/*========== Defines ==========*/

int iDebug;
char sPathFile[MAX_PATHFILE + 2];
int iNoAudio;
int iScale;
int iFullscreen;
int iStartLevel;
int iNoController;
int iBrokenRoomLinks;
int iDone[24 + 2];
int iCurRoom;
SDL_Window *window;
SDL_Window *windowmap;
unsigned int iWindowID;
unsigned int iWindowMapID;
SDL_Renderer *ascreen;
SDL_Renderer *mscreen;
unsigned int iActiveWindowID;
SDL_Cursor *curArrow;
SDL_Cursor *curWait;
SDL_Cursor *curHand;
int iHourglassFrame;
int iSandFrame;
int iPreLoaded;
int iNrToPreLoad;
int iCurrentBarHeight;
TTF_Font *font11;
TTF_Font *font15;
TTF_Font *font20;
int iStatusBarFrame;
int iScreen;
int iXPos, iYPos;
int iOKOn;
int iChanged;
int iCurLevel;
char cCurType;
int iCurGuard;
int iDownAt;
int iSelected;
int iExtras;
int iLastThing;
int iLastModifier;
int iCloseOn;
int iCustomOn;
int iIsCustom;
int iOnTile;
int iOnTileOld;
int iChangeEvent;
int iGuardSkill;
int iNoAnim;
int iFlameFrameDP;
int iChomperFrameDP;
int iSwordFrameDP;
int iMovingRoom;
int iMovingNewBusy;
int iChangingBrokenRoom;
int iChangingBrokenSide;
int iRoomArray[24 + 1 + 2][24 + 2];
int iMapOpen;
int iMovingOldX;
int iMovingOldY;
int iMovingNewX;
int iMovingNewY;
int iZoom; /*** For the big area on the Map window. ***/
int iXPosMapMoveOffset, iYPosMapMoveOffset;
int iMovingMap;
int iMapHoverYes;
int iMapHoverRoom;
int iXPosMap, iYPosMap;
int iMapHoverTile;
int iMapShowNumbers;
int iMapMoved;
int iDownAtMap;
int iXPosMapMoveStart, iYPosMapMoveStart;
int iMouse;
int iEventHover;
int iHelpOK;
int iChangeGroup, iChangeVariant;
int iCustomOK;
int iInfo;
int iThingACopyPaste[30 + 2];
int iModifierACopyPaste[30 + 2];
int iGuardACopyPaste[4 + 2];
int iCopied;
char sStatus[MAX_STATUS + 2], sStatusOld[MAX_STATUS + 2];
unsigned char cStore[1 + 2];
int iModified;
int iPlaytest;
int iYesOn, iNoOn;

int iEXESave;
int iEXEMinutesLeft;
int iEXEHitPoints;
int iEXESkipPotions;
int iEXEWinRoom;
int iEXEStartLevel;

/*** This is a level. ***/
unsigned char sLevelForeground[(ROOMS * TILES) + 2];
	int iThingA[ROOMS + 2][TILES + 2];
unsigned char sLevelModifier[(ROOMS * TILES) + 2];
	int iModifierA[ROOMS + 2][TILES + 2];
unsigned char sFirstDoorEvents[EVENTS + 2];
unsigned char sSecondDoorEvents[EVENTS + 2];
unsigned char sRoomLinks[(ROOMS * 4) + 2];
unsigned char sUnknownI[64 + 2];
unsigned char sStartPosition[3 + 2];
unsigned char sUnknownIIandIII[21 + 2];
unsigned char sGuardLocations[ROOMS + 2];
unsigned char sGuardDirections[ROOMS + 2];
unsigned char sUnknownIVaandIVb[48 + 2];
unsigned char sGuardSkills[ROOMS + 2];
unsigned char sUnknownIVc[24 + 2];
unsigned char sGuardColors[ROOMS + 2];
unsigned char sUnknownIVd[4 + 2];

int iRoomConnections[ROOMS + 2][4 + 2];
unsigned long luKidRoom;
unsigned long luKidPos;
unsigned long luKidDir;

int iRoomConnectionsBroken[ROOMS + 2][4 + 2];
int iMinX, iMaxX, iMinY, iMaxY, iStartRoomsX, iStartRoomsY;

unsigned int gamespeed;
Uint32 looptime;

static const int arDefaultEnv[16] =
	{ 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0 };
static const int arDefaultGRes[16] =
	{ 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 4, 3, 65535, 65535 };

static const int broken_room_x = 435;
static const int broken_room_y = 78;
static const int broken_side_x[5] = { 0, 420, 450, 435, 435 };
static const int broken_side_y[5] = { 0, 78, 78, 63, 93 };

/*** controller ***/
int iController;
SDL_GameController *controller;
char sControllerName[MAX_CON + 2];
SDL_Joystick *joystick;
SDL_Haptic *haptic;
Uint32 joyleft;
Uint32 joyright;
Uint32 joyup;
Uint32 joydown;
Uint32 trigleft;
Uint32 trigright;

/*** for text ***/
SDL_Surface *message;
SDL_Texture *messaget;
SDL_Rect offset;
SDL_Color color_bl = {0x00, 0x00, 0x00, 255};
SDL_Color color_wh = {0xff, 0xff, 0xff, 255};
SDL_Color color_blue = {0x00, 0x00, 0xff, 255};
SDL_Color color_f4 = {0xf4, 0xf4, 0xf4, 255};

SDL_Texture *imgloading;
SDL_Texture *imghourglasssprite;
SDL_Texture *imgsandsprite;
SDL_Texture *imgblack;
SDL_Texture *imgfaded;
SDL_Texture *imgpopup;
SDL_Texture *imgok[2 + 2];
SDL_Texture *imgdungeon[255 + 2][255 + 2];
SDL_Texture *imgpalace[255 + 2][255 + 2];
SDL_Texture *imgselected[255 + 2][255 + 2];
SDL_Texture *imgleft_0, *imgleft_1;
SDL_Texture *imgright_0, *imgright_1;
SDL_Texture *imgup_0, *imgup_1;
SDL_Texture *imgdown_0, *imgdown_1;
SDL_Texture *imglrno, *imgudno, *imgudnonfo;
SDL_Texture *imgroomsoff, *imgroomson_0, *imgroomson_1;
SDL_Texture *imgbroomsoff, *imgbroomson_0, *imgbroomson_1;
SDL_Texture *imgeventsoff, *imgeventson_0, *imgeventson_1;
SDL_Texture *imgsaveoff, *imgsaveon_0, *imgsaveon_1;
SDL_Texture *imgquit_0, *imgquit_1;
SDL_Texture *imgprevoff, *imgprevon_0, *imgprevon_1;
SDL_Texture *imgnextoff, *imgnexton_0, *imgnexton_1;
SDL_Texture *imgkidr[2 + 2], *imgkidl[2 + 2];
SDL_Texture *imgbar;
SDL_Texture *extras[10 + 2];
SDL_Texture *imgdungeont, *imgpalacet;
SDL_Texture *imgclosebig_0, *imgclosebig_1;
SDL_Texture *imgcustoma0, *imgcustomi0, *imgcustoma1, *imgcustomi1;
SDL_Texture *imgborderb, *imgborders, *imgborderbl, *imgbordersl;
SDL_Texture *imgnoliving;
SDL_Texture *imgfadeds;
SDL_Texture *imgsell;
SDL_Texture *imgrl, *imgbrl;
SDL_Texture *imgroom[24 + 2];
SDL_Texture *imgsrc, *imgsrs, *imgsrm, *imgsrp, *imgsrb;
SDL_Texture *imgevents, *imgsele, *imgeventu;
SDL_Texture *imgmouse;
SDL_Texture *imgeventh;
SDL_Texture *imghelp;
SDL_Texture *imgcustom;
SDL_Texture *imgsave[2 + 2];
SDL_Texture *imgunk[2 + 2];
SDL_Texture *imgexe;
SDL_Texture *imgstatusbarsprite;
SDL_Texture *imgplaytest;
SDL_Texture *imgpopup_yn;
SDL_Texture *imgyes[2 + 2], *imgno[2 + 2];

/*** Map window ***/
SDL_Texture *imgmapon_0, *imgmapon_1, *imgmapoff;
SDL_Texture *imggrid;
SDL_Texture *imgmap, *imgmapgrid;
SDL_Texture *imgmini[31 + 2][255 + 2];
SDL_Texture *imgminiguard, *imgminiprince, *imgminihover, *imgminirelated;
SDL_Texture *imgclose[2 + 2];
SDL_Texture *imgzoom1on_0, *imgzoom1on_1, *imgzoom1off;
SDL_Texture *imgzoomfiton_0, *imgzoomfiton_1, *imgzoomfitoff;
SDL_Texture *imgzoominon_0, *imgzoominon_1, *imgzoominoff;
SDL_Texture *imgzoomouton_0, *imgzoomouton_1, *imgzoomoutoff;
SDL_Texture *imgarrowdoff, *imgarrowdon_0, *imgarrowdon_1;
SDL_Texture *imgarrowloff, *imgarrowlon_0, *imgarrowlon_1;
SDL_Texture *imgarrowroff, *imgarrowron_0, *imgarrowron_1;
SDL_Texture *imgarrowuoff, *imgarrowuon_0, *imgarrowuon_1;
SDL_Texture *imgbmrooma, *imgbmroomh;
SDL_Texture *imgsellm;

/*** sprites ***/
SDL_Texture *spriteflamed1;
SDL_Texture *spriteflamed2;
SDL_Texture *spriteflamep1;
SDL_Texture *spriteflamep2;
SDL_Texture *spritechomperd1;
SDL_Texture *spritechomperd2;
SDL_Texture *spritechomperp1;
SDL_Texture *spritechomperp2;
SDL_Texture *spritechompersel2;
SDL_Texture *spritechompersel1;
SDL_Texture *spriteswordd;
SDL_Texture *spriteswordp;
SDL_Texture *spriteswordsel;

/*** guards ***/
SDL_Texture *imggblush[2 + 2];
SDL_Texture *imggyellow[2 + 2];
SDL_Texture *imggrouge[2 + 2];
SDL_Texture *imggrose[2 + 2];
SDL_Texture *imggdgreen[2 + 2];
SDL_Texture *imggblue[2 + 2];
SDL_Texture *imgglgreen[2 + 2];
SDL_Texture *imggunknown[2 + 2];
SDL_Texture *imgskel[2 + 2];
SDL_Texture *imgfat[2 + 2];
SDL_Texture *imgjaffar[2 + 2];
SDL_Texture *imgshadow[2 + 2];
/***/
SDL_Texture *imggsel[2 + 2];
SDL_Texture *imgskelsel[2 + 2];
SDL_Texture *imgfatsel[2 + 2];
SDL_Texture *imgjaffarsel[2 + 2];
SDL_Texture *imgshadowsel[2 + 2];

struct sample {
	Uint8 *data;
	Uint32 dpos;
	Uint32 dlen;
} sounds[NUM_SOUNDS];

void ShowUsage (void);
void PrIfDe (char *sString);
void GetPathFile (void);
void GetOptionValue (char *sArgv, char *sValue);
void LoadLevel (int iLevel);
void SaveLevel (int iLevel);
int ReadFromFile (int iFd, char *sWhat, int iSize, unsigned char *sRetString);
int EventInfo (int iNr, int iType);
void GetAsEightBits (unsigned char cChar, char *sBinary);
int BitsToInt (char *sString);
char cShowDirection (int iDirection);
int BrokenRoomLinks (int iPrint);
void CheckSides (int iRoom, int iX, int iY);
void InitScreenAction (char *sAction);
void InitScreen (void);
void LoadFonts (void);
void MixAudio (void *unused, Uint8 *stream, int iLen);
void PreLoad (char *sPath, char *sPNG, SDL_Texture **imgImage);
void PreLoadMap (char *sPath, char *sPNG, SDL_Texture **imgImage);
void ShowImage (SDL_Texture *img, int iX, int iY, char *sImageInfo,
	SDL_Renderer *screen, float fMultiply, int iXYScale);
void LoadingBar (int iBarHeight);
void ShowScreen (int iScreenS);
void InitPopUp (void);
void ShowPopUp (void);
int MapEvents (SDL_Event event);
void Quit (void);
void PlaySound (char *sFile);
int InArea (int iUpperLeftX, int iUpperLeftY,
	int iLowerRightX, int iLowerRightY);
int InAreaMap (int iUpperLeftX, int iUpperLeftY,
	int iLowerRightX, int iLowerRightY);
void InitPopUpSave (void);
void ShowPopUpSave (void);
void DisplayText (int iStartX, int iStartY, int iFontSize,
	char arText[9 + 2][MAX_TEXT + 2], int iLines, TTF_Font *font,
	SDL_Color back, SDL_Color fore, int iOnMap);
void CustomRenderCopy (SDL_Texture* src, char *sImageInfo, SDL_Rect* srcrect,
	SDL_Renderer* dst, SDL_Rect *dstrect);
void GetForegroundAsName (char *sBinaryFore, char *sName);
void ShowTile (int iThing, int iModifier, int iLocation);
void LSeekLevel (int iFd, int iLevel);
void WriteCharByChar (int iFd, unsigned char *sString, int iLength);
void Prev (void);
void Next (void);
void LocationToXY (int iLocation, int iXY[4]);
void ShowMap (void);
void SetLocation (int iRoom, int iLocation, int iThing, int iModifier);
void ChangePosAction (char *sAction);
void ChangePos (int iLocation);
void ShowChange (int iLocation);
void UseTile (int iTile, int iLocation, int iRoom);
int IsDisabled (int iTile);
int OnTile (void);
void SetGuard (int iLoc, int iDirection, int iSkill, int iColor);
void DisableSome (void);
int OnTileOld (void);
void CenterNumber (SDL_Renderer *screen, int iNumber, int iX, int iY,
	SDL_Color fore, int iHex);
void InitRooms (void);
void WhereToStart (void);
void ShowRooms (int iRoom, int iX, int iY, int iNext);
int MouseSelectAdj (void);
void RemoveOldRoom (void);
void AddNewRoom (int iX, int iY, int iRoom);
void MapShow (void);
void LinkPlus (void);
void LinkMinus (void);
void ClearRoom (void);
float ZoomGet (void);
int MapGridStartX (void);
int MapGridStartY (void);
void SetMapHover (int iRoom, int iX, int iY);
void ShowRoomsMap (int iRoom, int iX, int iY);
int RelatedToHover (int iRoom, int iTile);
void MapAction (char *sAction);
void MapControllerDown (SDL_Event event);
void MapControllerUp (SDL_Event event);
void MapControllerMotion (SDL_Event event);
void MapButtonDown (SDL_Event event);
void MapButtonUp (SDL_Event event);
void MapKeyDown (SDL_Event event);
void MapMouseMotion (SDL_Event event);
void MapMouseWheel (SDL_Event event);
void MapHide (void);
void ZoomIncrease (void);
void ZoomDecrease (void);
void ChangeEvent (int iAmount, int iChangePos);
void EventRoom (int iRoom);
void EventDoor (int iTile);
void EventNext (int iNoNext);
void Help (void);
void ShowHelp (void);
void OpenURL (char *sURL);
void ApplySkillIfNecessary (int iLoc);
void ChangePosCustomAction (char *sAction, int iLocation);
int ChangePosCustom (int iLocation);
void ChangeCustom (int iAmount, char cGroupOrVariant);
void ShowChangeCustom (void);
void CopyPaste (int iRoom, int iAction);
void Zoom (int iToggleFull);
void EXE (void);
void ShowEXE (void);
void EXELoad (void);
void EXESave (void);
void UpdateStatusBar (void);
int PlusMinus (int *iWhat, int iX, int iY,
	int iMin, int iMax, int iChange, int iAddChanged);
unsigned long BytesAsLU (unsigned char *sData, int iBytes);
void WriteByte (int iFd, int iValue);
void WriteWord (int iFd, int iValue);
void CreateBAK (void);
int OnLevelBar (void);
void RunLevel (int iLevel);
int StartGame (void *unused);
void ModifyForPlaytest (int iLevel);
void ModifyBack (void);
void Sprinkle (void);
void FlipRoom (int iRoom, int iAxis);
void GetModifierAsName (int iFore, int iMod, char *sName);

/*****************************************************************************/
int main (int argc, char *argv[])
/*****************************************************************************/
{
	int iLoopArg;
	char sStartLevel[MAX_OPTION + 2];
	time_t tm;
	SDL_version verc, verl;

	iDebug = 0;
	iNoAudio = 0;
	iScale = 1;
	iFullscreen = 0;
	iStartLevel = 1;
	iNoController = 0;
	iExtras = 0;
	iLastThing = 0;
	iLastModifier = 0;
	iNoAnim = 0;
	iMapOpen = 0;
	iZoom = DEFAULT_ZOOM;
	iMapHoverRoom = 0;
	iMapShowNumbers = 0;
	iMouse = 0;
	iEventHover = 0;
	iInfo = 0;
	iCopied = 0;
	iModified = 0;

	if (argc > 1)
	{
		for (iLoopArg = 1; iLoopArg <= argc - 1; iLoopArg++)
		{
			if ((strcmp (argv[iLoopArg], "-h") == 0) ||
				(strcmp (argv[iLoopArg], "-?") == 0) ||
				(strcmp (argv[iLoopArg], "--help") == 0))
			{
				ShowUsage();
			}
			else if ((strcmp (argv[iLoopArg], "-v") == 0) ||
				(strcmp (argv[iLoopArg], "--version") == 0))
			{
				printf ("%s %s\n", EDITOR_NAME, EDITOR_VERSION);
				exit (EXIT_NORMAL);
			}
			else if ((strcmp (argv[iLoopArg], "-d") == 0) ||
				(strcmp (argv[iLoopArg], "--debug") == 0))
			{
				iDebug = 1;
			}
			else if ((strcmp (argv[iLoopArg], "-n") == 0) ||
				(strcmp (argv[iLoopArg], "--noaudio") == 0))
			{
				iNoAudio = 1;
			}
			else if ((strcmp (argv[iLoopArg], "-z") == 0) ||
				(strcmp (argv[iLoopArg], "--zoom") == 0))
			{
				iScale = 2;
			}
			else if ((strcmp (argv[iLoopArg], "-f") == 0) ||
				(strcmp (argv[iLoopArg], "--fullscreen") == 0))
			{
				iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
			}
			else if ((strncmp (argv[iLoopArg], "-l=", 3) == 0) ||
				(strncmp (argv[iLoopArg], "--level=", 8) == 0))
			{
				GetOptionValue (argv[iLoopArg], sStartLevel);
				iStartLevel = atoi (sStartLevel);
				if ((iStartLevel < 0) || (iStartLevel > MAX_LEVEL))
				{
					iStartLevel = 1;
				}
			}
			else if ((strcmp (argv[iLoopArg], "-s") == 0) ||
				(strcmp (argv[iLoopArg], "--static") == 0))
			{
				iNoAnim = 1;
			}
			else if ((strcmp (argv[iLoopArg], "-k") == 0) ||
				(strcmp (argv[iLoopArg], "--keyboard") == 0))
			{
				iNoController = 1;
			}
			else
			{
				ShowUsage();
			}
		}
	}

	GetPathFile();

	srand ((unsigned)time(&tm));

	LoadLevel (iStartLevel);

	/*** Show the SDL version used for compiling and linking. ***/
	if (iDebug == 1)
	{
		SDL_VERSION (&verc);
		SDL_GetVersion (&verl);
		printf ("[ INFO ] Compiled with SDL %u.%u.%u, linked with SDL %u.%u.%u.\n",
			verc.major, verc.minor, verc.patch, verl.major, verl.minor, verl.patch);
	}

	InitScreen();

	Quit();

	return 0;
}
/*****************************************************************************/
void ShowUsage (void)
/*****************************************************************************/
{
	printf ("%s %s\n%s\n\n", EDITOR_NAME, EDITOR_VERSION, COPYRIGHT);
	printf ("Usage:\n");
	printf ("  %s [OPTIONS]\n\nOptions:\n", EDITOR_NAME);
	printf ("  -h, -?,    --help           display this help and exit\n");
	printf ("  -v,        --version        output version information and"
		" exit\n");
	printf ("  -d,        --debug          also show levels on the console\n");
	printf ("  -n,        --noaudio        do not play sound effects\n");
	printf ("  -z,        --zoom           double the interface size\n");
	printf ("  -f,        --fullscreen     start in fullscreen mode\n");
	printf ("  -l=NR,     --level=NR       start in level NR\n");
	printf ("  -s,        --static         do not display animations\n");
	printf ("  -k,        --keyboard       do not use a game controller\n");
	printf ("\n");
	exit (EXIT_NORMAL);
}
/*****************************************************************************/
void PrIfDe (char *sString)
/*****************************************************************************/
{
	if (iDebug == 1) { printf ("%s", sString); }
}
/*****************************************************************************/
void GetPathFile (void)
/*****************************************************************************/
{
	int iFound;
	DIR *dDir;
	struct dirent *stDirent;
	char sExtension[100 + 2];

	iFound = 0;

	dDir = opendir (BIN_DIR);
	if (dDir == NULL)
	{
		printf ("[FAILED] Cannot open directory \"%s\": %s!\n",
			BIN_DIR, strerror (errno));
		exit (EXIT_ERROR);
	}

	while ((stDirent = readdir (dDir)) != NULL)
	{
		if (iFound == 0)
		{
			if ((strcmp (stDirent->d_name, ".") != 0) &&
				(strcmp (stDirent->d_name, "..") != 0))
			{
				snprintf (sExtension, 100, "%s", strrchr (stDirent->d_name, '.'));
				if ((toupper (sExtension[1]) == 'B') &&
					(toupper (sExtension[2]) == 'I') &&
					(toupper (sExtension[3]) == 'N'))
				{
					iFound = 1;
					snprintf (sPathFile, MAX_PATHFILE, "%s%s%s", BIN_DIR, SLASH,
						stDirent->d_name);
					if (iDebug == 1)
					{
						printf ("[  OK  ] Found Macintosh BIN \"%s\".\n", sPathFile);
					}
				}
			}
		}
	}
}
/*****************************************************************************/
void GetOptionValue (char *sArgv, char *sValue)
/*****************************************************************************/
{
	int iTemp;
	char sTemp[MAX_OPTION + 2];

	iTemp = strlen (sArgv) - 1;
	snprintf (sValue, MAX_OPTION, "%s", "");
	while (sArgv[iTemp] != '=')
	{
		snprintf (sTemp, MAX_OPTION, "%c%s", sArgv[iTemp], sValue);
		snprintf (sValue, MAX_OPTION, "%s", sTemp);
		iTemp--;
	}
}
/*****************************************************************************/
void LoadLevel (int iLevel)
/*****************************************************************************/
{
	int iFd;
	int iPosFore, iPosBack;
	char sBinaryFore[9 + 2]; /*** 8 chars, plus \0 ***/
	char sName[7 + 2];
	char sForegrounds[100 + 2], sForegroundsTemp[100 + 2];
	char sModifiers[100 + 2], sModifiersTemp[100 + 2];
	int iNameDone;
	char sString[MAX_DATA + 2];
	char sReferenceNr[7 + 2];

	/*** Used for looping. ***/
	int iLoopRoom;
	int iLoopTile;
	int iLoopEvent;
	int iLoopLink;
	int iLoopMinus;
	int iLoopGuard;

	iFd = open (sPathFile, O_RDONLY|O_BINARY);
	if (iFd == -1)
	{
		printf ("[FAILED] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	LSeekLevel (iFd, iLevel);

	iChanged = 0;

	ReadFromFile (iFd, "Level Foreground", ROOMS * TILES, sLevelForeground);
	ReadFromFile (iFd, "Level Modifier", ROOMS * TILES, sLevelModifier);
	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
	{
		for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{
			iPosFore = ((iLoopRoom - 1) * TILES) + (iLoopTile - 1);
			iPosBack = iPosFore;

			iThingA[iLoopRoom][iLoopTile] = sLevelForeground[iPosFore];
			iModifierA[iLoopRoom][iLoopTile] = sLevelModifier[iPosBack];
		}
	}
	if (iDebug == 1)
	{
		snprintf (sForegrounds, 100, "%s", "");
		snprintf (sModifiers, 100, "%s", "");
		for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
		{
			printf ("\nRoom %i:\n\n", iLoopRoom);
			for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
			{
				iPosFore = ((iLoopRoom - 1) * TILES) + (iLoopTile - 1);
				iPosBack = iPosFore;

				GetAsEightBits (sLevelForeground[iPosFore], sBinaryFore);
				GetForegroundAsName (sBinaryFore, sName);

				if (strcmp (sForegrounds, "") == 0)
				{
					snprintf (sForegrounds, 100, "%s", sName);
				} else {
					snprintf (sForegroundsTemp, 100, "%s", sForegrounds);
					snprintf (sForegrounds, 100, "%s|%s", sForegroundsTemp, sName);
				}

				iNameDone = 0;

				/*** gate or door ***/
				if ((iThingA[iLoopRoom][iLoopTile] == 0x04) ||
					(iThingA[iLoopRoom][iLoopTile] == (0x04 + 0x20)))
				{
					switch (sLevelModifier[iPosBack])
					{
						case 1: snprintf (sName, 9, "%s", "Open   "); break;
						default: snprintf (sName, 9, "%s", "Closed "); break;
					}
					iNameDone = 1;
				}

				/*** drop or raise ***/
				if ((iThingA[iLoopRoom][iLoopTile] == 0x06) ||
					(iThingA[iLoopRoom][iLoopTile] == 0x0F) ||
					(iThingA[iLoopRoom][iLoopTile] == (0x06 + 0x20)) ||
					(iThingA[iLoopRoom][iLoopTile] == (0x0F + 0x20)))
				{
					snprintf (sString, 3 + 6, "%i      ",
						sLevelModifier[iPosBack] + 1);
					strncpy (sReferenceNr, sString, 7);
					sReferenceNr[7] = '\0';
					snprintf (sName, 9, "%s", sReferenceNr);
					iNameDone = 1;
				}

				/*** the rest ***/
				if (iNameDone == 0)
				{
					GetModifierAsName (sLevelForeground[iPosFore],
						sLevelModifier[iPosBack], sName);
				}

				if (strcmp (sModifiers, "") == 0)
				{
					snprintf (sModifiers, 100, "%s", sName);
				} else {
					snprintf (sModifiersTemp, 100, "%s", sModifiers);
					snprintf (sModifiers, 100, "%s|%s", sModifiersTemp, sName);
				}

				if ((iLoopTile == 10) || (iLoopTile == 20) || (iLoopTile == 30))
				{
					printf ("%s\n", sForegrounds);
					printf ("%s\n", sModifiers);
					if ((iLoopTile == 10) || (iLoopTile == 20))
					{
						for (iLoopMinus = 1; iLoopMinus <= 79; iLoopMinus++)
							{ printf ("-"); }
						printf ("\n");
					}
					snprintf (sForegrounds, 100, "%s", "");
					snprintf (sModifiers, 100, "%s", "");
				}
			}
			PrIfDe ("\n");
		}
	}

	/*** Load first and second door events. ***/
	ReadFromFile (iFd, "First Door Events", EVENTS, sFirstDoorEvents);
	ReadFromFile (iFd, "Second Door Events", EVENTS, sSecondDoorEvents);
	if (iDebug == 1)
	{
		for (iLoopEvent = 0; iLoopEvent < EVENTS; iLoopEvent++)
		{
			printf ("[ INFO ] Event %i changes the door in room: %i, location:"
				" %i.",
				iLoopEvent + 1,
				EventInfo (iLoopEvent, 1),
				EventInfo (iLoopEvent, 2));
			switch (EventInfo (iLoopEvent, 3))
			{
				case 1: printf (" (next: yes)\n"); break;
				case 0: printf (" (next: no)\n"); break;
			}
		}
	}

	/*** Load the room links. ***/
	ReadFromFile (iFd, "Room Links", (ROOMS * 4), sRoomLinks);
	for (iLoopLink = 0; iLoopLink < (ROOMS * 4); iLoopLink+=4)
	{
		iRoomConnections[(iLoopLink / 4) + 1][1] =
			sRoomLinks[iLoopLink]; /*** left ***/
		iRoomConnections[(iLoopLink / 4) + 1][2] =
			sRoomLinks[iLoopLink + 1]; /*** right ***/
		iRoomConnections[(iLoopLink / 4) + 1][3] =
			sRoomLinks[iLoopLink + 2]; /*** up ***/
		iRoomConnections[(iLoopLink / 4) + 1][4] =
			sRoomLinks[iLoopLink + 3]; /*** down ***/
		if (iDebug == 1)
		{
			printf ("[ INFO ] Room %i is connected to room (0 = none): l%i, "
				"r%i, u%i, d%i\n",
				(iLoopLink / 4) + 1,
				sRoomLinks[iLoopLink],
				sRoomLinks[iLoopLink + 1],
				sRoomLinks[iLoopLink + 2],
				sRoomLinks[iLoopLink + 3]);
		}
	}

	/*** Load Unknown I. ***/
	ReadFromFile (iFd, "Unknown I", 64, sUnknownI);
	/*** We want remapped modifiers in all 24 rooms. ***/
	sUnknownI[0] = 0x18;

	/*** Load Start Position. ***/
	ReadFromFile (iFd, "Start Position", 3, sStartPosition);
	luKidRoom = sStartPosition[0];
	luKidPos = sStartPosition[1] + 1;
	luKidDir = sStartPosition[2];
	/*** 1 of 2 ***/
	if ((iLevel == 1) || (iLevel == 13))
	{
		if (luKidDir == 0) { luKidDir = 255; } else { luKidDir = 0; }
	}
	if (iDebug == 1)
	{
		printf ("[ INFO ] The kid starts in room: %lu, position: %lu, turned: "
			"%c\n",
			luKidRoom,
			luKidPos,
			cShowDirection ((int)luKidDir));
	}

	PrIfDe ("[  OK  ] Checking for broken room links.\n");
	iBrokenRoomLinks = BrokenRoomLinks (1);

	/*** Load Unknown II-IVd, guard data. ***/
	ReadFromFile (iFd, "Unknown II and III", 21, sUnknownIIandIII);
	ReadFromFile (iFd, "Guard Locations", ROOMS, sGuardLocations);
	ReadFromFile (iFd, "Guard Directions", ROOMS, sGuardDirections);
	ReadFromFile (iFd, "Unknown IVa and IVb", 48, sUnknownIVaandIVb);
	ReadFromFile (iFd, "Guard Skills", ROOMS, sGuardSkills);
	ReadFromFile (iFd, "Unknown IVc", 24, sUnknownIVc);
	ReadFromFile (iFd, "Guard Colors", ROOMS, sGuardColors);
	ReadFromFile (iFd, "Unknown IVd", 4, sUnknownIVd);

	/*** Display guard information. ***/
	if (iDebug == 1)
	{
		for (iLoopGuard = 0; iLoopGuard < ROOMS; iLoopGuard++)
		{
			if (sGuardLocations[iLoopGuard] < 30)
			{
				printf ("[ INFO ] A guard in room: %i, location: %i, turned: %c, "
					"skill: %i, color: %i\n",
					iLoopGuard + 1,
					sGuardLocations[iLoopGuard] + 1,
					cShowDirection (sGuardDirections[iLoopGuard]),
					sGuardSkills[iLoopGuard],
					sGuardColors[iLoopGuard]);
			}
		}
	}

	switch (arDefaultEnv[iLevel])
	{
		case 0: cCurType = 'd'; break;
		case 1: cCurType = 'p'; break;
	}
	iCurGuard = arDefaultGRes[iLevel];

	/*** Back to a centered map. ***/
	iXPosMapMoveOffset = 0;
	iYPosMapMoveOffset = 0;

	iCurLevel = iLevel;
	iCurRoom = (int)luKidRoom;
	iOnTile = 1;
	iPlaytest = 0;
	close (iFd);
}
/*****************************************************************************/
void SaveLevel (int iLevel)
/*****************************************************************************/
{
	int iFd;
	char sToWrite[MAX_TOWRITE + 2];

	/*** Used for looping. ***/
	int iLoopRoom;
	int iLoopTile;
	int iLoopLink;

	if (iChanged == 0) { return; }

	CreateBAK();

	iFd = open (sPathFile, O_WRONLY|O_BINARY, 0600);
	if (iFd == -1)
	{
		printf ("[FAILED] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	LSeekLevel (iFd, iLevel);

	/*** Level Foreground ***/
	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
	{
		for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{
			snprintf (sToWrite, MAX_TOWRITE, "%c", iThingA[iLoopRoom][iLoopTile]);
			write (iFd, sToWrite, 1);
		}
	}

	/*** Level Modifier ***/
	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
	{
		for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{
			snprintf (sToWrite, MAX_TOWRITE, "%c", iModifierA[iLoopRoom][iLoopTile]);
			write (iFd, sToWrite, 1);
		}
	}

	WriteCharByChar (iFd, sFirstDoorEvents, 256);
	WriteCharByChar (iFd, sSecondDoorEvents, 256);

	for (iLoopLink = 1; iLoopLink <= (ROOMS * 4); iLoopLink++)
	{
		snprintf (sToWrite, MAX_TOWRITE, "%c",
			iRoomConnections[((int)((iLoopLink - 1) / 4)) + 1]
			[((iLoopLink - 1) % 4) + 1]);
		write (iFd, sToWrite, 1);
	}

	WriteCharByChar (iFd, sUnknownI, 64);

	/*** prince ***/
	snprintf (sToWrite, MAX_TOWRITE, "%c", (int)luKidRoom);
	write (iFd, sToWrite, 1);
	snprintf (sToWrite, MAX_TOWRITE, "%c", (int)luKidPos - 1);
	write (iFd, sToWrite, 1);
	snprintf (sToWrite, MAX_TOWRITE, "%c", (int)luKidDir);
	/*** 2 of 2 ***/
	if ((iCurLevel == 1) || (iCurLevel == 13))
	{
		if (luKidDir == 0) { snprintf (sToWrite, MAX_TOWRITE, "%c", 255); }
			else { snprintf (sToWrite, MAX_TOWRITE, "%c", 0); }
	}
	write (iFd, sToWrite, 1);

	WriteCharByChar (iFd, sUnknownIIandIII, 21);
	WriteCharByChar (iFd, sGuardLocations, 24);
	WriteCharByChar (iFd, sGuardDirections, 24);
	WriteCharByChar (iFd, sUnknownIVaandIVb, 48);
	WriteCharByChar (iFd, sGuardSkills, 24);
	WriteCharByChar (iFd, sUnknownIVc, 24);
	WriteCharByChar (iFd, sGuardColors, 24);
	WriteCharByChar (iFd, sUnknownIVd, 4);

	close (iFd);

	PlaySound ("wav/save.wav");

	iChanged = 0;
}
/*****************************************************************************/
int ReadFromFile (int iFd, char *sWhat, int iSize, unsigned char *sRetString)
/*****************************************************************************/
{
	int iLength;
	int iRead;
	char sRead[1 + 2];
	int iEOF;

	if ((iDebug == 1) && (strcmp (sWhat, "") != 0))
	{
		printf ("[  OK  ] Loading: %s\n", sWhat);
	}
	iLength = 0;
	iEOF = 0;
	do {
		iRead = read (iFd, sRead, 1);
		switch (iRead)
		{
			case -1:
				printf ("[FAILED] Could not read: %s!\n", strerror (errno));
				exit (EXIT_ERROR);
				break;
			case 0: PrIfDe ("[ INFO ] End of file\n"); iEOF = 1; break;
			default:
				sRetString[iLength] = sRead[0];
				iLength++;
				break;
		}
	} while ((iLength < iSize) && (iEOF == 0));
	sRetString[iLength] = '\0';

	return (iLength);
}
/*****************************************************************************/
int EventInfo (int iNr, int iType)
/*****************************************************************************/
{
	char sBinaryFDoors[9 + 2]; /*** 8 chars, plus \0 ***/
	char sBinarySDoors[9 + 2]; /*** 8 chars, plus \0 ***/
	char sTemp[10 + 2];

	GetAsEightBits (sFirstDoorEvents[iNr], sBinaryFDoors);
	GetAsEightBits (sSecondDoorEvents[iNr], sBinarySDoors);

	switch (iType)
	{
		case 1: /*** room ***/
			snprintf (sTemp, 10, "%c%c%c%c%c", sBinarySDoors[0],
				sBinarySDoors[1], sBinarySDoors[2], sBinaryFDoors[1],
				sBinaryFDoors[2]);
			return (BitsToInt (sTemp));
			break;
		case 2: /*** location ***/
			snprintf (sTemp, 10, "%c%c%c%c%c", sBinaryFDoors[3],
				sBinaryFDoors[4], sBinaryFDoors[5], sBinaryFDoors[6],
				sBinaryFDoors[7]);
			return (BitsToInt (sTemp) + 1); break;
		case 3: /*** next ***/
			switch (sBinaryFDoors[0])
			{
				case '0': return (1); break;
				case '1': return (0); break;
			}
			break;
		default:
			printf ("[ WARN ] Strange event info type: %i!\n", iType);
			break;
	}

	return (-1);
}
/*****************************************************************************/
void GetAsEightBits (unsigned char cChar, char *sBinary)
/*****************************************************************************/
{
	int i = CHAR_BIT;
	int iTemp;

	iTemp = 0;
	while (i > 0)
	{
		i--;
		if (cChar&(1 << i))
		{
			sBinary[iTemp] = '1';
		} else {
			sBinary[iTemp] = '0';
		}
		iTemp++;
	}
	sBinary[iTemp] = '\0';
}
/*****************************************************************************/
int BitsToInt (char *sString)
/*****************************************************************************/
{
	/*** Converts binary to decimal. ***/
	/*** Example: 11111111 to 255 ***/

	int iTemp = 0;

	for (; *sString; iTemp = (iTemp << 1) | (*sString++ - '0'));
	return (iTemp);
}
/*****************************************************************************/
char cShowDirection (int iDirection)
/*****************************************************************************/
{
	switch (iDirection)
	{
		case 0: return ('r'); break;
		case 255: return ('l'); break;
		default: return ('?'); break;
	}
}
/*****************************************************************************/
int BrokenRoomLinks (int iPrint)
/*****************************************************************************/
{
	int iBroken;
	int iTemp;

	for (iTemp = 1; iTemp <= ROOMS; iTemp++)
	{
		iDone[iTemp] = 0;
		iRoomConnectionsBroken[iTemp][1] = 0;
		iRoomConnectionsBroken[iTemp][2] = 0;
		iRoomConnectionsBroken[iTemp][3] = 0;
		iRoomConnectionsBroken[iTemp][4] = 0;
	}
	CheckSides ((int)luKidRoom, 0, 0);
	iBroken = 0;

	for (iTemp = 1; iTemp <= ROOMS; iTemp++)
	{
		/*** If the room is in use... ***/
		if (iDone[iTemp] == 1)
		{
			/*** check left ***/
			if (iRoomConnections[iTemp][1] != 0)
			{
				if ((iRoomConnections[iTemp][1] == iTemp) ||
					(iRoomConnections[iRoomConnections[iTemp][1]][2] != iTemp))
				{
					iRoomConnectionsBroken[iTemp][1] = 1;
					iBroken = 1;
					if ((iDebug == 1) && (iPrint == 1))
					{
						printf ("[ INFO ] The left of room %i has a broken link.\n",
							iTemp);
					}
				}
			}
			/*** check right ***/
			if (iRoomConnections[iTemp][2] != 0)
			{
				if ((iRoomConnections[iTemp][2] == iTemp) ||
					(iRoomConnections[iRoomConnections[iTemp][2]][1] != iTemp))
				{
					iRoomConnectionsBroken[iTemp][2] = 1;
					iBroken = 1;
					if ((iDebug == 1) && (iPrint == 1))
					{
						printf ("[ INFO ] The right of room %i has a broken link.\n",
							iTemp);
					}
				}
			}
			/*** check up ***/
			if (iRoomConnections[iTemp][3] != 0)
			{
				if ((iRoomConnections[iTemp][3] == iTemp) ||
					(iRoomConnections[iRoomConnections[iTemp][3]][4] != iTemp))
				{
					iRoomConnectionsBroken[iTemp][3] = 1;
					iBroken = 1;
					if ((iDebug == 1) && (iPrint == 1))
					{
						printf ("[ INFO ] The top of room %i has a broken link.\n",
							iTemp);
					}
				}
			}
			/*** check down ***/
			if (iRoomConnections[iTemp][4] != 0)
			{
				if ((iRoomConnections[iTemp][4] == iTemp) ||
					(iRoomConnections[iRoomConnections[iTemp][4]][3] != iTemp))
				{
					iRoomConnectionsBroken[iTemp][4] = 1;
					iBroken = 1;
					if ((iDebug == 1) && (iPrint == 1))
					{
						printf ("[ INFO ] The bottom of room %i has a broken link.\n",
							iTemp);
					}
				}
			}
		}
	}

	if (iBroken == 1) { MapHide(); }

	return (iBroken);
}
/*****************************************************************************/
void CheckSides (int iRoom, int iX, int iY)
/*****************************************************************************/
{
	if (iX < iMinX) { iMinX = iX; }
	if (iY < iMinY) { iMinY = iY; }
	if (iX > iMaxX) { iMaxX = iX; }
	if (iY > iMaxY) { iMaxY = iY; }

	iDone[iRoom] = 1;

	if ((iRoomConnections[iRoom][1] != 0) &&
		(iDone[iRoomConnections[iRoom][1]] != 1))
		{ CheckSides (iRoomConnections[iRoom][1], iX - 1, iY); }

	if ((iRoomConnections[iRoom][2] != 0) &&
		(iDone[iRoomConnections[iRoom][2]] != 1))
		{ CheckSides (iRoomConnections[iRoom][2], iX + 1, iY); }

	if ((iRoomConnections[iRoom][3] != 0) &&
		(iDone[iRoomConnections[iRoom][3]] != 1))
		{ CheckSides (iRoomConnections[iRoom][3], iX, iY - 1); }

	if ((iRoomConnections[iRoom][4] != 0) &&
		(iDone[iRoomConnections[iRoom][4]] != 1))
		{ CheckSides (iRoomConnections[iRoom][4], iX, iY + 1); }
}
/*****************************************************************************/
void InitScreenAction (char *sAction)
/*****************************************************************************/
{
	int iEventDoor;
	int iEventRoom;

	if (strcmp (sAction, "left") == 0)
	{
		switch (iScreen)
		{
			case 1:
				iSelected--;
				switch (iSelected)
				{
					case 0: iSelected = 10; break;
					case 10: iSelected = 20; break;
					case 20: iSelected = 30; break;
				}
				break;
			case 2:
				if (iBrokenRoomLinks == 0)
				{
					if (iMovingRoom != 0)
					{
						if (iMovingNewX != 1) { iMovingNewX--; }
							else { iMovingNewX = 25; }
						PlaySound ("wav/cross.wav");
					}
				} else {
					if (iChangingBrokenSide != 1)
					{
						iChangingBrokenSide = 1;
					} else {
						switch (iChangingBrokenRoom)
						{
							case 1: iChangingBrokenRoom = 4; break;
							case 5: iChangingBrokenRoom = 8; break;
							case 9: iChangingBrokenRoom = 12; break;
							case 13: iChangingBrokenRoom = 16; break;
							case 17: iChangingBrokenRoom = 20; break;
							case 21: iChangingBrokenRoom = 24; break;
							default: iChangingBrokenRoom--; break;
						}
					}
				}
				break;
			case 3:
				iEventDoor = EventInfo (iChangeEvent, 2);
				switch (iEventDoor)
				{
					case 1: iEventDoor = 10; break;
					case 11: iEventDoor = 20; break;
					case 21: iEventDoor = 30; break;
					default: iEventDoor--; break;
				}
				EventDoor (iEventDoor);
				break;
		}
	}

	if (strcmp (sAction, "right") == 0)
	{
		switch (iScreen)
		{
			case 1:
				iSelected++;
				switch (iSelected)
				{
					case 11: iSelected = 1; break;
					case 21: iSelected = 11; break;
					case 31: iSelected = 21; break;
				}
				break;
			case 2:
				if (iBrokenRoomLinks == 0)
				{
					if (iMovingRoom != 0)
					{
						if (iMovingNewX != 25) { iMovingNewX++; }
							else { iMovingNewX = 1; }
						PlaySound ("wav/cross.wav");
					}
				} else {
					if (iChangingBrokenSide != 2)
					{
						iChangingBrokenSide = 2;
					} else {
						switch (iChangingBrokenRoom)
						{
							case 4: iChangingBrokenRoom = 1; break;
							case 8: iChangingBrokenRoom = 5; break;
							case 12: iChangingBrokenRoom = 9; break;
							case 16: iChangingBrokenRoom = 13; break;
							case 20: iChangingBrokenRoom = 17; break;
							case 24: iChangingBrokenRoom = 21; break;
							default: iChangingBrokenRoom++; break;
						}
					}
				}
				break;
			case 3:
				iEventDoor = EventInfo (iChangeEvent, 2);
				switch (iEventDoor)
				{
					case 10: iEventDoor = 1; break;
					case 20: iEventDoor = 11; break;
					case 30: iEventDoor = 21; break;
					default: iEventDoor++; break;
				}
				EventDoor (iEventDoor);
				break;
		}
	}

	if (strcmp (sAction, "up") == 0)
	{
		switch (iScreen)
		{
			case 1:
				if (iSelected > 10) { iSelected-=10; }
					else { iSelected+=20; }
				break;
			case 2:
				if (iBrokenRoomLinks == 0)
				{
					if (iMovingRoom != 0)
					{
						if (iMovingNewY != 1) { iMovingNewY--; }
							else { iMovingNewY = 24; }
						PlaySound ("wav/cross.wav");
					}
				} else {
					if (iChangingBrokenSide != 3)
					{
						iChangingBrokenSide = 3;
					} else {
						switch (iChangingBrokenRoom)
						{
							case 1: iChangingBrokenRoom = 21; break;
							case 2: iChangingBrokenRoom = 22; break;
							case 3: iChangingBrokenRoom = 23; break;
							case 4: iChangingBrokenRoom = 24; break;
							default: iChangingBrokenRoom -= 4; break;
						}
					}
				}
				break;
			case 3:
				iEventDoor = EventInfo (iChangeEvent, 2);
				if (iEventDoor > 10) { iEventDoor-=10; }
					else { iEventDoor+=20; }
				EventDoor (iEventDoor);
				break;
		}
	}

	if (strcmp (sAction, "down") == 0)
	{
		switch (iScreen)
		{
			case 1:
				if (iSelected <= 20) { iSelected+=10; }
					else { iSelected-=20; }
				break;
			case 2:
				if (iBrokenRoomLinks == 0)
				{
					if (iMovingRoom != 0)
					{
						if (iMovingNewY != 24) { iMovingNewY++; }
							else { iMovingNewY = 1; }
						PlaySound ("wav/cross.wav");
					}
				} else {
					if (iChangingBrokenSide != 4)
					{
						iChangingBrokenSide = 4;
					} else {
						switch (iChangingBrokenRoom)
						{
							case 21: iChangingBrokenRoom = 1; break;
							case 22: iChangingBrokenRoom = 2; break;
							case 23: iChangingBrokenRoom = 3; break;
							case 24: iChangingBrokenRoom = 4; break;
							default: iChangingBrokenRoom += 4; break;
						}
					}
				}
				break;
			case 3:
				iEventDoor = EventInfo (iChangeEvent, 2);
				if (iEventDoor <= 20) { iEventDoor+=10; }
					else { iEventDoor-=20; }
				EventDoor (iEventDoor);
				break;
		}
	}

	if (strcmp (sAction, "left bracket") == 0)
	{
		switch (iScreen)
		{
			case 2:
				if (iBrokenRoomLinks == 0)
				{
					iMovingNewBusy = 0;
					switch (iMovingRoom)
					{
						case 0: iMovingRoom = ROOMS; break;
						case 1: iMovingRoom = ROOMS; break;
						default: iMovingRoom--; break;
					}
				}
				break;
			case 3:
				iEventRoom = EventInfo (iChangeEvent, 1);
				if ((iEventRoom >= 2) && (iEventRoom <= 24))
				{
					EventRoom (iEventRoom - 1);
				} else {
					EventRoom (24);
				}
				break;
		}
	}

	if (strcmp (sAction, "right bracket") == 0)
	{
		switch (iScreen)
		{
			case 2:
				if (iBrokenRoomLinks == 0)
				{
					iMovingNewBusy = 0;
					switch (iMovingRoom)
					{
						case 0: iMovingRoom = 1; break;
						case 24: iMovingRoom = 1; break;
						default: iMovingRoom++; break;
					}
				}
				break;
			case 3:
				iEventRoom = EventInfo (iChangeEvent, 1);
				if ((iEventRoom >= 1) && (iEventRoom <= 23))
				{
					EventRoom (iEventRoom + 1);
				} else {
					EventRoom (1);
				}
				break;
		}
	}

	if (strcmp (sAction, "enter") == 0)
	{
		switch (iScreen)
		{
			case 1:
				ChangePos (iSelected);
				break;
			case 2:
				if (iBrokenRoomLinks == 0)
				{
					if (iMovingRoom != 0)
					{
						if (iRoomArray[iMovingNewX][iMovingNewY] == 0)
						{
							RemoveOldRoom();
							AddNewRoom (iMovingNewX, iMovingNewY, iMovingRoom);
							iChanged++;
						}
						iMovingRoom = 0; iMovingNewBusy = 0;
					}
				} else {
					LinkPlus();
				}
				break;
		}
	}

	if (strcmp (sAction, "env") == 0)
	{
		/*** Nothing for now. ***/
	}
}
/*****************************************************************************/
void InitScreen (void)
/*****************************************************************************/
{
	SDL_AudioSpec fmt;
	char sImage[MAX_IMG + 2];
	SDL_Surface *imgicon;
	int iJoyNr;
	SDL_Rect barbox;
	SDL_Event event;
	DIR *dDir;
	struct dirent *stDirent;
	char sThing[3 + 2]; /*** 2 chars, plus \0 ***/
	char sModifier[3 + 2]; /*** 2 chars, plus \0 ***/
	int iXPosOld, iYPosOld;
	int iLocX, iLocY;
	char sExtra[20 + 2];
	char sRoom[20 + 2];
	const Uint8 *keystate;
	Uint32 oldticks, newticks;
	int iXJoy1, iYJoy1, iXJoy2, iYJoy2;

	/*** Used for looping. ***/
	int iLoopTile;
	int iLoopExtra;
	int iLoopRoom;
	int iLoopX, iLoopY;

	if (SDL_Init (SDL_INIT_AUDIO|SDL_INIT_VIDEO|
		SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC) < 0)
	{
		printf ("[FAILED] Unable to init SDL: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	atexit (SDL_Quit);

	/*** main window ***/
	window = SDL_CreateWindow (EDITOR_NAME " " EDITOR_VERSION,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(WINDOW_WIDTH) * iScale, (WINDOW_HEIGHT) * iScale, iFullscreen);
	if (window == NULL)
	{
		printf ("[FAILED] Unable to create main window: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	iWindowID = SDL_GetWindowID (window);
	iActiveWindowID = iWindowID;
	ascreen = SDL_CreateRenderer (window, -1, 0);
	if (ascreen == NULL)
	{
		printf ("[FAILED] Unable to set video mode: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}

	/*** Map window ***/
	windowmap = SDL_CreateWindow ("Map", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 1291, 858, SDL_WINDOW_HIDDEN);
	if (windowmap == NULL)
	{
		printf ("[FAILED] Unable to create Map window: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	iWindowMapID = SDL_GetWindowID (windowmap);
	mscreen = SDL_CreateRenderer (windowmap, -1, 0);
	if (mscreen == NULL)
	{
		printf ("[FAILED] Unable to set video mode: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}

	/*** Some people may prefer linear, but we're going old school. ***/
	SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	if (iFullscreen != 0)
	{
		SDL_RenderSetLogicalSize (ascreen, (WINDOW_WIDTH) * iScale,
			(WINDOW_HEIGHT) * iScale);
	}

	if (TTF_Init() == -1)
	{
		exit (EXIT_ERROR);
	}

	LoadFonts();

	curArrow = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_ARROW);
	curWait = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_WAIT);
	curHand = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_HAND);

	if (iNoAudio != 1)
	{
		PrIfDe ("[  OK  ] Initializing Audio\n");
		fmt.freq = 44100;
		fmt.format = AUDIO_S16;
		fmt.channels = 2;
		fmt.samples = 512;
		fmt.callback = MixAudio;
		fmt.userdata = NULL;
		if (SDL_OpenAudio (&fmt, NULL) < 0)
		{
			printf ("[FAILED] Unable to open audio: %s!\n", SDL_GetError());
			exit (EXIT_ERROR);
		}
		SDL_PauseAudio (0);
	}

	/*** main window icon ***/
	snprintf (sImage, MAX_IMG, "%s%s", PNG_VARIOUS, "popyorn_icon.png");
	imgicon = IMG_Load (sImage);
	if (imgicon == NULL)
	{
		printf ("[ WARN ] Could not load \"%s\": %s!\n", sImage, strerror (errno));
	} else {
		SDL_SetWindowIcon (window, imgicon);
	}

	/*** Map window icon ***/
	snprintf (sImage, MAX_IMG, "%s%s", PNG_VARIOUS, "map_icon.png");
	imgicon = IMG_Load (sImage);
	if (imgicon == NULL)
	{
		printf ("[ WARN ] Could not load \"%s\": %s!\n", sImage, strerror (errno));
	} else {
		SDL_SetWindowIcon (windowmap, imgicon);
	}

	/*** Open the first available controller. ***/
	iController = 0;
	if (iNoController != 1)
	{
		for (iJoyNr = 0; iJoyNr < SDL_NumJoysticks(); iJoyNr++)
		{
			if (SDL_IsGameController (iJoyNr))
			{
				controller = SDL_GameControllerOpen (iJoyNr);
				if (controller)
				{
					snprintf (sControllerName, MAX_CON, "%s",
						SDL_GameControllerName (controller));
					if (iDebug == 1)
					{
						printf ("[ INFO ] Found a controller \"%s\"; \"%s\".\n",
							sControllerName, SDL_GameControllerNameForIndex (iJoyNr));
					}
					joystick = SDL_GameControllerGetJoystick (controller);
					iController = 1;

					/*** Just for fun, use haptic. ***/
					if (SDL_JoystickIsHaptic (joystick))
					{
						haptic = SDL_HapticOpenFromJoystick (joystick);
						if (SDL_HapticRumbleInit (haptic) == 0)
						{
							SDL_HapticRumblePlay (haptic, 1.0, 1000);
						} else {
							printf ("[ WARN ] Could not initialize the haptic device: %s\n",
								SDL_GetError());
						}
					} else {
						PrIfDe ("[ INFO ] The game controller is not haptic.\n");
					}
				} else {
					printf ("[ WARN ] Could not open game controller %i: %s\n",
						iController, SDL_GetError());
				}
			}
		}
		if (iController != 1) { PrIfDe ("[ INFO ] No controller found.\n"); }
	} else {
		PrIfDe ("[ INFO ] Using keyboard and mouse.\n");
	}

	/*******************/
	/* Preload images. */
	/*******************/

	/*** loading ***/
	SDL_SetCursor (curWait);
	PreLoad (PNG_VARIOUS, "loading.png", &imgloading);
	PreLoad (PNG_VARIOUS, "hourglass_sprite.png", &imghourglasssprite);
	PreLoad (PNG_VARIOUS, "sand_sprite.png", &imgsandsprite);
	ShowImage (imgloading, 0, 0, "imgloading", ascreen, iScale, 1);
	SDL_SetRenderDrawColor (ascreen, 0x22, 0x22, 0x22, SDL_ALPHA_OPAQUE);
	barbox.x = 10 * iScale;
	barbox.y = 10 * iScale;
	barbox.w = 20 * iScale;
	barbox.h = 439 * iScale;
	SDL_RenderFillRect (ascreen, &barbox);
	iHourglassFrame = 1;
	iSandFrame = 1;
	SDL_RenderPresent (ascreen);

	iPreLoaded = 0;
	iCurrentBarHeight = 0;
	iNrToPreLoad = 531; /*** Value can be obtained via debug mode. ***/
	SDL_SetCursor (curWait);

	/*** buttons ***/
	PreLoad (PNG_BUTTONS, "left_0.png", &imgleft_0);
	PreLoad (PNG_BUTTONS, "left_1.png", &imgleft_1);
	PreLoad (PNG_BUTTONS, "right_0.png", &imgright_0);
	PreLoad (PNG_BUTTONS, "right_1.png", &imgright_1);
	PreLoad (PNG_BUTTONS, "up_0.png", &imgup_0);
	PreLoad (PNG_BUTTONS, "up_1.png", &imgup_1);
	PreLoad (PNG_BUTTONS, "down_0.png", &imgdown_0);
	PreLoad (PNG_BUTTONS, "down_1.png", &imgdown_1);
	PreLoad (PNG_BUTTONS, "left_right_no.png", &imglrno);
	PreLoad (PNG_BUTTONS, "up_down_no.png", &imgudno);
	PreLoad (PNG_BUTTONS, "custom_active_0.png", &imgcustoma0);
	PreLoad (PNG_BUTTONS, "custom_inactive_0.png", &imgcustomi0);
	PreLoad (PNG_BUTTONS, "custom_active_1.png", &imgcustoma1);
	PreLoad (PNG_BUTTONS, "custom_inactive_1.png", &imgcustomi1);
	if (iController == 0)
	{
		PreLoad (PNG_BUTTONS, "OK.png", &imgok[1]);
		PreLoad (PNG_BUTTONS, "sel_OK.png", &imgok[2]);
		PreLoad (PNG_BUTTONS, "close_big_0.png", &imgclosebig_0);
		PreLoad (PNG_BUTTONS, "close_big_1.png", &imgclosebig_1);
		PreLoad (PNG_BUTTONS, "rooms_on_0.png", &imgroomson_0);
		PreLoad (PNG_BUTTONS, "rooms_on_1.png", &imgroomson_1);
		PreLoad (PNG_BUTTONS, "rooms_off.png", &imgroomsoff);
		PreLoad (PNG_BUTTONS, "broken_rooms_on_0.png", &imgbroomson_0);
		PreLoad (PNG_BUTTONS, "broken_rooms_on_1.png", &imgbroomson_1);
		PreLoad (PNG_BUTTONS, "broken_rooms_off.png", &imgbroomsoff);
		PreLoad (PNG_BUTTONS, "events_on_0.png", &imgeventson_0);
		PreLoad (PNG_BUTTONS, "events_on_1.png", &imgeventson_1);
		PreLoad (PNG_BUTTONS, "events_off.png", &imgeventsoff);
		PreLoad (PNG_BUTTONS, "save_on_0.png", &imgsaveon_0);
		PreLoad (PNG_BUTTONS, "save_on_1.png", &imgsaveon_1);
		PreLoad (PNG_BUTTONS, "save_off.png", &imgsaveoff);
		PreLoad (PNG_BUTTONS, "quit_0.png", &imgquit_0);
		PreLoad (PNG_BUTTONS, "quit_1.png", &imgquit_1);
		PreLoad (PNG_BUTTONS, "prev_on_0.png", &imgprevon_0);
		PreLoad (PNG_BUTTONS, "prev_on_1.png", &imgprevon_1);
		PreLoad (PNG_BUTTONS, "prev_off.png", &imgprevoff);
		PreLoad (PNG_BUTTONS, "next_on_0.png", &imgnexton_0);
		PreLoad (PNG_BUTTONS, "next_on_1.png", &imgnexton_1);
		PreLoad (PNG_BUTTONS, "next_off.png", &imgnextoff);
		PreLoad (PNG_BUTTONS, "up_down_no_nfo.png", &imgudnonfo);
		PreLoad (PNG_BUTTONS, "Save.png", &imgsave[1]);
		PreLoad (PNG_BUTTONS, "sel_Save.png", &imgsave[2]);
		PreLoad (PNG_BUTTONS, "Yes.png", &imgyes[1]);
		PreLoad (PNG_BUTTONS, "sel_Yes.png", &imgyes[2]);
		PreLoad (PNG_BUTTONS, "No.png", &imgno[1]);
		PreLoad (PNG_BUTTONS, "sel_No.png", &imgno[2]);
	} else {
		PreLoad (PNG_GAMEPAD, "OK.png", &imgok[1]);
		PreLoad (PNG_GAMEPAD, "sel_OK.png", &imgok[2]);
		PreLoad (PNG_GAMEPAD, "close_big_0.png", &imgclosebig_0);
		PreLoad (PNG_GAMEPAD, "close_big_1.png", &imgclosebig_1);
		PreLoad (PNG_GAMEPAD, "rooms_on_0.png", &imgroomson_0);
		PreLoad (PNG_GAMEPAD, "rooms_on_1.png", &imgroomson_1);
		PreLoad (PNG_GAMEPAD, "rooms_off.png", &imgroomsoff);
		PreLoad (PNG_GAMEPAD, "broken_rooms_on_0.png", &imgbroomson_0);
		PreLoad (PNG_GAMEPAD, "broken_rooms_on_1.png", &imgbroomson_1);
		PreLoad (PNG_GAMEPAD, "broken_rooms_off.png", &imgbroomsoff);
		PreLoad (PNG_GAMEPAD, "events_on_0.png", &imgeventson_0);
		PreLoad (PNG_GAMEPAD, "events_on_1.png", &imgeventson_1);
		PreLoad (PNG_GAMEPAD, "events_off.png", &imgeventsoff);
		PreLoad (PNG_GAMEPAD, "save_on_0.png", &imgsaveon_0);
		PreLoad (PNG_GAMEPAD, "save_on_1.png", &imgsaveon_1);
		PreLoad (PNG_GAMEPAD, "save_off.png", &imgsaveoff);
		PreLoad (PNG_GAMEPAD, "quit_0.png", &imgquit_0);
		PreLoad (PNG_GAMEPAD, "quit_1.png", &imgquit_1);
		PreLoad (PNG_GAMEPAD, "prev_on_0.png", &imgprevon_0);
		PreLoad (PNG_GAMEPAD, "prev_on_1.png", &imgprevon_1);
		PreLoad (PNG_GAMEPAD, "prev_off.png", &imgprevoff);
		PreLoad (PNG_GAMEPAD, "next_on_0.png", &imgnexton_0);
		PreLoad (PNG_GAMEPAD, "next_on_1.png", &imgnexton_1);
		PreLoad (PNG_GAMEPAD, "next_off.png", &imgnextoff);
		PreLoad (PNG_GAMEPAD, "up_down_no_nfo.png", &imgudnonfo);
		PreLoad (PNG_GAMEPAD, "Save.png", &imgsave[1]);
		PreLoad (PNG_GAMEPAD, "sel_Save.png", &imgsave[2]);
		PreLoad (PNG_GAMEPAD, "Yes.png", &imgyes[1]);
		PreLoad (PNG_GAMEPAD, "sel_Yes.png", &imgyes[2]);
		PreLoad (PNG_GAMEPAD, "No.png", &imgno[1]);
		PreLoad (PNG_GAMEPAD, "sel_No.png", &imgno[2]);
	}

	/*** living ***/
	PreLoad (PNG_LIVING, "kid_r.png", &imgkidr[1]);
	PreLoad (PNG_LIVING, "kid_l.png", &imgkidl[1]);
	PreLoad (PNG_LIVING, "guard_blush_l.png", &imggblush[1]);
	PreLoad (PNG_LIVING, "guard_blush_r.png", &imggblush[2]);
	PreLoad (PNG_LIVING, "guard_yellow_l.png", &imggyellow[1]);
	PreLoad (PNG_LIVING, "guard_yellow_r.png", &imggyellow[2]);
	PreLoad (PNG_LIVING, "guard_rouge_l.png", &imggrouge[1]);
	PreLoad (PNG_LIVING, "guard_rouge_r.png", &imggrouge[2]);
	PreLoad (PNG_LIVING, "guard_rose_l.png", &imggrose[1]);
	PreLoad (PNG_LIVING, "guard_rose_r.png", &imggrose[2]);
	PreLoad (PNG_LIVING, "guard_dgreen_l.png", &imggdgreen[1]);
	PreLoad (PNG_LIVING, "guard_dgreen_r.png", &imggdgreen[2]);
	PreLoad (PNG_LIVING, "guard_blue_l.png", &imggblue[1]);
	PreLoad (PNG_LIVING, "guard_blue_r.png", &imggblue[2]);
	PreLoad (PNG_LIVING, "guard_lgreen_l.png", &imgglgreen[1]);
	PreLoad (PNG_LIVING, "guard_lgreen_r.png", &imgglgreen[2]);
	PreLoad (PNG_LIVING, "guard_unknown_l.png", &imggunknown[1]);
	PreLoad (PNG_LIVING, "guard_unknown_r.png", &imggunknown[2]);
	PreLoad (PNG_LIVING, "skeleton_l.png", &imgskel[1]);
	PreLoad (PNG_LIVING, "skeleton_r.png", &imgskel[2]);
	PreLoad (PNG_LIVING, "fat_l.png", &imgfat[1]);
	PreLoad (PNG_LIVING, "fat_r.png", &imgfat[2]);
	PreLoad (PNG_LIVING, "jaffar_l.png", &imgjaffar[1]);
	PreLoad (PNG_LIVING, "jaffar_r.png", &imgjaffar[2]);
	PreLoad (PNG_LIVING, "shadow_l.png", &imgshadow[1]);
	PreLoad (PNG_LIVING, "shadow_r.png", &imgshadow[2]);

	/*** sliving ***/
	PreLoad (PNG_SLIVING, "kid_r.png", &imgkidr[2]);
	PreLoad (PNG_SLIVING, "kid_l.png", &imgkidl[2]);
	PreLoad (PNG_SLIVING, "guard_l.png", &imggsel[1]);
	PreLoad (PNG_SLIVING, "guard_r.png", &imggsel[2]);
	PreLoad (PNG_SLIVING, "skeleton_l.png", &imgskelsel[1]);
	PreLoad (PNG_SLIVING, "skeleton_r.png", &imgskelsel[2]);
	PreLoad (PNG_SLIVING, "fat_l.png", &imgfatsel[1]);
	PreLoad (PNG_SLIVING, "fat_r.png", &imgfatsel[2]);
	PreLoad (PNG_SLIVING, "jaffar_l.png", &imgjaffarsel[1]);
	PreLoad (PNG_SLIVING, "jaffar_r.png", &imgjaffarsel[2]);
	PreLoad (PNG_SLIVING, "shadow_l.png", &imgshadowsel[1]);
	PreLoad (PNG_SLIVING, "shadow_r.png", &imgshadowsel[2]);

	/*** various ***/
	PreLoad (PNG_VARIOUS, "black.png", &imgblack);
	PreLoad (PNG_VARIOUS, "faded.png", &imgfaded);
	PreLoad (PNG_VARIOUS, "border_big_live.png", &imgborderbl);
	PreLoad (PNG_VARIOUS, "border_small_live.png", &imgbordersl);
	PreLoad (PNG_VARIOUS, "no_living.png", &imgnoliving);
	PreLoad (PNG_VARIOUS, "faded_s.png", &imgfadeds);
	PreLoad (PNG_VARIOUS, "sel_level.png", &imgsell);
	PreLoad (PNG_VARIOUS, "sel_room_current.png", &imgsrc);
	PreLoad (PNG_VARIOUS, "sel_room_start.png", &imgsrs);
	PreLoad (PNG_VARIOUS, "sel_room_moving.png", &imgsrm);
	PreLoad (PNG_VARIOUS, "sel_room_cross.png", &imgsrp);
	PreLoad (PNG_VARIOUS, "sel_room_broken.png", &imgsrb);
	PreLoad (PNG_VARIOUS, "sel_event.png", &imgsele);
	PreLoad (PNG_VARIOUS, "event_unused.png", &imgeventu);
	PreLoad (PNG_VARIOUS, "mouse.png", &imgmouse);
	PreLoad (PNG_VARIOUS, "event_hover.png", &imgeventh);
	PreLoad (PNG_VARIOUS, "help.png", &imghelp);
	PreLoad (PNG_VARIOUS, "unknown.png", &imgunk[1]);
	PreLoad (PNG_VARIOUS, "sel_unknown.png", &imgunk[2]);
	PreLoad (PNG_VARIOUS, "exe.png", &imgexe);
	PreLoad (PNG_VARIOUS, "statusbar_sprite.png", &imgstatusbarsprite);
	PreLoad (PNG_VARIOUS, "playtest.png", &imgplaytest);
	PreLoad (PNG_VARIOUS, "popup_yn.png", &imgpopup_yn);
	if (iController == 0)
	{
		PreLoad (PNG_VARIOUS, "dungeon.png", &imgdungeont);
		PreLoad (PNG_VARIOUS, "palace.png", &imgpalacet);
		PreLoad (PNG_VARIOUS, "popup.png", &imgpopup);
		PreLoad (PNG_VARIOUS, "level_bar.png", &imgbar);
		PreLoad (PNG_VARIOUS, "border_big.png", &imgborderb);
		PreLoad (PNG_VARIOUS, "border_small.png", &imgborders);
		PreLoad (PNG_VARIOUS, "room_links.png", &imgrl);
		PreLoad (PNG_VARIOUS, "broken_room_links.png", &imgbrl);
		PreLoad (PNG_VARIOUS, "events.png", &imgevents);
		PreLoad (PNG_VARIOUS, "custom.png", &imgcustom);
	} else {
		PreLoad (PNG_GAMEPAD, "dungeon.png", &imgdungeont);
		PreLoad (PNG_GAMEPAD, "palace.png", &imgpalacet);
		PreLoad (PNG_GAMEPAD, "popup.png", &imgpopup);
		PreLoad (PNG_GAMEPAD, "level_bar.png", &imgbar);
		PreLoad (PNG_GAMEPAD, "border_big.png", &imgborderb);
		PreLoad (PNG_GAMEPAD, "border_small.png", &imgborders);
		PreLoad (PNG_GAMEPAD, "room_links.png", &imgrl);
		PreLoad (PNG_GAMEPAD, "broken_room_links.png", &imgbrl);
		PreLoad (PNG_GAMEPAD, "events.png", &imgevents);
		PreLoad (PNG_GAMEPAD, "custom.png", &imgcustom);
	}

	/*** imgdungeon ***/
	dDir = opendir (PNG_DUNGEON);
	if (dDir == NULL)
	{
		printf ("[FAILED] Cannot open directory \"%s\": %s!\n",
			PNG_DUNGEON, strerror (errno));
		exit (EXIT_ERROR);
	}
	while ((stDirent = readdir (dDir)) != NULL)
	{
		if ((strcmp (stDirent->d_name, ".") != 0) &&
			(strcmp (stDirent->d_name, "..") != 0))
		{
			snprintf (sThing, 3, "%c%c",
				stDirent->d_name[0], stDirent->d_name[1]);
			snprintf (sModifier, 3, "%c%c",
				stDirent->d_name[3], stDirent->d_name[4]);
			PreLoad (PNG_DUNGEON, stDirent->d_name,
				&imgdungeon[(int)strtol(sThing, NULL, 16)]
				[(int)strtol(sModifier, NULL, 16)]);
		}
	}
	closedir (dDir);
	PreLoad (PNG_DUNGEON, "13_00_sprite.png", &spriteflamed1);
	PreLoad (PNG_DUNGEON, "1E_00_sprite.png", &spriteflamed2);
	PreLoad (PNG_DUNGEON, "12_00_sprite.png", &spritechomperd1);
	PreLoad (PNG_DUNGEON, "12_80_sprite.png", &spritechomperd2);
	PreLoad (PNG_SELECTED, "12_00_sprite.png", &spritechompersel1);
	PreLoad (PNG_SELECTED, "12_80_sprite.png", &spritechompersel2);
	PreLoad (PNG_DUNGEON, "16_00_sprite.png", &spriteswordd);
	PreLoad (PNG_SELECTED, "16_00_sprite.png", &spriteswordsel);

	/*** imgpalace ***/
	dDir = opendir (PNG_PALACE);
	if (dDir == NULL)
	{
		printf ("[FAILED] Cannot open directory \"%s\": %s!\n",
			PNG_PALACE, strerror (errno));
		exit (EXIT_ERROR);
	}
	while ((stDirent = readdir (dDir)) != NULL)
	{
		if ((strcmp (stDirent->d_name, ".") != 0) &&
			(strcmp (stDirent->d_name, "..") != 0))
		{
			snprintf (sThing, 3, "%c%c",
				stDirent->d_name[0], stDirent->d_name[1]);
			snprintf (sModifier, 3, "%c%c",
				stDirent->d_name[3], stDirent->d_name[4]);
			PreLoad (PNG_PALACE, stDirent->d_name,
				&imgpalace[(int)strtol(sThing, NULL, 16)]
				[(int)strtol(sModifier, NULL, 16)]);
		}
	}
	closedir (dDir);
	PreLoad (PNG_PALACE, "13_00_sprite.png", &spriteflamep1);
	PreLoad (PNG_PALACE, "1E_00_sprite.png", &spriteflamep2);
	PreLoad (PNG_PALACE, "12_00_sprite.png", &spritechomperp1);
	PreLoad (PNG_PALACE, "12_80_sprite.png", &spritechomperp2);
	/*** + spritechompersel1/2 (see dungeon) ***/
	PreLoad (PNG_PALACE, "16_00_sprite.png", &spriteswordp);
	/*** + spriteswordsel (see dungeon) ***/

	/*** imgselected ***/
	dDir = opendir (PNG_SELECTED);
	if (dDir == NULL)
	{
		printf ("[FAILED] Cannot open directory \"%s\": %s!\n",
			PNG_SELECTED, strerror (errno));
		exit (EXIT_ERROR);
	}
	while ((stDirent = readdir (dDir)) != NULL)
	{
		if ((strcmp (stDirent->d_name, ".") != 0) &&
			(strcmp (stDirent->d_name, "..") != 0))
		{
			snprintf (sThing, 3, "%c%c",
				stDirent->d_name[0], stDirent->d_name[1]);
			snprintf (sModifier, 3, "%c%c",
				stDirent->d_name[3], stDirent->d_name[4]);
			PreLoad (PNG_SELECTED, stDirent->d_name,
				&imgselected[(int)strtol(sThing, NULL, 16)]
				[(int)strtol(sModifier, NULL, 16)]);
		}
	}
	closedir (dDir);

	/*** extras ***/
	for (iLoopExtra = 0; iLoopExtra <= 10; iLoopExtra++)
	{
		snprintf (sExtra, 20, "extras_%02i.png", iLoopExtra);
		PreLoad (PNG_EXTRAS, sExtra, &extras[iLoopExtra]);
	}

	/*** 14x14 rooms ***/
	for (iLoopRoom = 1; iLoopRoom <= 24; iLoopRoom++)
	{
		snprintf (sRoom, 20, "room%i.png", iLoopRoom);
		PreLoad (PNG_ROOMS, sRoom, &imgroom[iLoopRoom]);
	}

	/*** Map window ***/
	PreLoadMap (PNG_VARIOUS, "map.png", &imgmap);
	PreLoadMap (PNG_VARIOUS, "map_grid.png", &imgmapgrid);
	PreLoadMap (PNG_MINI, "0.png", &imgmini[0][0]);
	PreLoadMap (PNG_MINI, "0_3.png", &imgmini[0][3]);
	PreLoadMap (PNG_MINI, "1.png", &imgmini[1][0]);
	PreLoadMap (PNG_MINI, "2.png", &imgmini[2][0]);
	PreLoadMap (PNG_MINI, "3.png", &imgmini[3][0]);
	PreLoadMap (PNG_MINI, "4_0.png", &imgmini[4][0]);
	PreLoadMap (PNG_MINI, "4_1.png", &imgmini[4][1]);
	PreLoadMap (PNG_MINI, "5.png", &imgmini[5][0]);
	PreLoadMap (PNG_MINI, "6.png", &imgmini[6][0]);
	PreLoadMap (PNG_MINI, "7.png", &imgmini[7][0]);
	PreLoadMap (PNG_MINI, "8.png", &imgmini[8][0]);
	PreLoadMap (PNG_MINI, "9.png", &imgmini[9][0]);
	PreLoadMap (PNG_MINI, "10_0.png", &imgmini[10][0]);
	PreLoadMap (PNG_MINI, "10_1.png", &imgmini[10][1]);
	PreLoadMap (PNG_MINI, "10_2.png", &imgmini[10][2]);
	PreLoadMap (PNG_MINI, "10_3.png", &imgmini[10][3]);
	PreLoadMap (PNG_MINI, "10_4.png", &imgmini[10][4]);
	PreLoadMap (PNG_MINI, "10_5.png", &imgmini[10][5]);
	PreLoadMap (PNG_MINI, "10_6.png", &imgmini[10][6]);
	PreLoadMap (PNG_MINI, "11.png", &imgmini[11][0]);
	PreLoadMap (PNG_MINI, "12.png", &imgmini[12][0]);
	PreLoadMap (PNG_MINI, "13.png", &imgmini[13][0]);
	PreLoadMap (PNG_MINI, "14.png", &imgmini[14][0]);
	PreLoadMap (PNG_MINI, "15.png", &imgmini[15][0]);
	PreLoadMap (PNG_MINI, "16.png", &imgmini[16][0]);
	PreLoadMap (PNG_MINI, "17.png", &imgmini[17][0]);
	PreLoadMap (PNG_MINI, "18.png", &imgmini[18][0]);
	PreLoadMap (PNG_MINI, "18_2.png", &imgmini[18][2]);
	PreLoadMap (PNG_MINI, "19.png", &imgmini[19][0]);
	PreLoadMap (PNG_MINI, "20.png", &imgmini[20][0]);
	PreLoadMap (PNG_MINI, "21.png", &imgmini[21][0]);
	PreLoadMap (PNG_MINI, "22.png", &imgmini[22][0]);
	PreLoadMap (PNG_MINI, "23.png", &imgmini[23][0]);
	PreLoadMap (PNG_MINI, "24.png", &imgmini[24][0]);
	PreLoadMap (PNG_MINI, "25.png", &imgmini[25][0]);
	PreLoadMap (PNG_MINI, "26.png", &imgmini[26][0]);
	PreLoadMap (PNG_MINI, "27.png", &imgmini[27][0]);
	PreLoadMap (PNG_MINI, "28.png", &imgmini[28][0]);
	PreLoadMap (PNG_MINI, "29.png", &imgmini[29][0]);
	PreLoadMap (PNG_MINI, "30.png", &imgmini[30][0]);
	PreLoadMap (PNG_MINI, "guard.png", &imgminiguard);
	PreLoadMap (PNG_MINI, "prince.png", &imgminiprince);
	PreLoadMap (PNG_MINI, "hover.png", &imgminihover);
	PreLoadMap (PNG_MINI, "related.png", &imgminirelated);
	PreLoadMap (PNG_BUTTONS, "zoom-1_on_0.png", &imgzoom1on_0);
	PreLoadMap (PNG_BUTTONS, "zoom-1_on_1.png", &imgzoom1on_1);
	PreLoadMap (PNG_BUTTONS, "zoom-1_off.png", &imgzoom1off);
	PreLoadMap (PNG_BUTTONS, "zoom-fit_on_0.png", &imgzoomfiton_0);
	PreLoadMap (PNG_BUTTONS, "zoom-fit_on_1.png", &imgzoomfiton_1);
	PreLoadMap (PNG_BUTTONS, "zoom-fit_off.png", &imgzoomfitoff);
	PreLoadMap (PNG_BUTTONS, "zoom-in_on_0.png", &imgzoominon_0);
	PreLoadMap (PNG_BUTTONS, "zoom-in_on_1.png", &imgzoominon_1);
	PreLoadMap (PNG_BUTTONS, "zoom-in_off.png", &imgzoominoff);
	PreLoadMap (PNG_BUTTONS, "zoom-out_on_0.png", &imgzoomouton_0);
	PreLoadMap (PNG_BUTTONS, "zoom-out_on_1.png", &imgzoomouton_1);
	PreLoadMap (PNG_BUTTONS, "zoom-out_off.png", &imgzoomoutoff);
	PreLoadMap (PNG_BUTTONS, "arrow_down_off.png", &imgarrowdoff);
	PreLoadMap (PNG_BUTTONS, "arrow_down_on_0.png", &imgarrowdon_0);
	PreLoadMap (PNG_BUTTONS, "arrow_down_on_1.png", &imgarrowdon_1);
	PreLoadMap (PNG_BUTTONS, "arrow_left_off.png", &imgarrowloff);
	PreLoadMap (PNG_BUTTONS, "arrow_left_on_0.png", &imgarrowlon_0);
	PreLoadMap (PNG_BUTTONS, "arrow_left_on_1.png", &imgarrowlon_1);
	PreLoadMap (PNG_BUTTONS, "arrow_right_off.png", &imgarrowroff);
	PreLoadMap (PNG_BUTTONS, "arrow_right_on_0.png", &imgarrowron_0);
	PreLoadMap (PNG_BUTTONS, "arrow_right_on_1.png", &imgarrowron_1);
	PreLoadMap (PNG_BUTTONS, "arrow_up_off.png", &imgarrowuoff);
	PreLoadMap (PNG_BUTTONS, "arrow_up_on_0.png", &imgarrowuon_0);
	PreLoadMap (PNG_BUTTONS, "arrow_up_on_1.png", &imgarrowuon_1);
	PreLoadMap (PNG_VARIOUS, "border_mroom_active.png", &imgbmrooma);
	PreLoadMap (PNG_VARIOUS, "border_mroom_hover.png", &imgbmroomh);
	PreLoadMap (PNG_VARIOUS, "sel_level.png", &imgsellm);
	if (iController != 1)
	{
		PreLoad (PNG_BUTTONS, "map_on_0.png", &imgmapon_0);
		PreLoad (PNG_BUTTONS, "map_on_1.png", &imgmapon_1);
		PreLoad (PNG_BUTTONS, "map_off.png", &imgmapoff);
		PreLoadMap (PNG_BUTTONS, "Close.png", &imgclose[1]);
		PreLoadMap (PNG_BUTTONS, "sel_Close.png", &imgclose[2]);
	} else {
		PreLoad (PNG_GAMEPAD, "map_on_0.png", &imgmapon_0);
		PreLoad (PNG_GAMEPAD, "map_on_1.png", &imgmapon_1);
		PreLoad (PNG_GAMEPAD, "map_off.png", &imgmapoff);
		PreLoadMap (PNG_GAMEPAD, "Close.png", &imgclose[1]);
		PreLoadMap (PNG_GAMEPAD, "sel_Close.png", &imgclose[2]);
	}

	/*** One final update of the bar. ***/
	LoadingBar (BAR_FULL);

	if (iDebug == 1)
		{ printf ("[ INFO ] Preloaded images: %i\n", iPreLoaded); }
	SDL_SetCursor (curArrow);

	/*** Start with a centered map. ***/
	iXPosMapMoveOffset = 0;
	iYPosMapMoveOffset = 0;
	iMovingMap = 0;

	iCurLevel = iStartLevel;
	if (iDebug == 1)
		{ printf ("[ INFO ] Starting in level: %i\n", iStartLevel); }

	iSelected = 1; /*** Start with the upper left selected. ***/
	iScreen = 1;
	iDownAt = 0;
	iChangeEvent = 0;
	iFlameFrameDP = 1;
	iChomperFrameDP = 1;
	iSwordFrameDP = 1;
	oldticks = 0;
	iDownAtMap = 0;

	ShowScreen (iScreen);
	InitPopUp();
	while (1)
	{
		if (iNoAnim == 0)
		{
			/*** This is for the animation; 12 fps (1000/83) is fine for PoP. ***/
			newticks = SDL_GetTicks();
			if (newticks > oldticks + 83)
			{
				iFlameFrameDP++;
				if (iFlameFrameDP == 10) { iFlameFrameDP = 1; }
				iChomperFrameDP++;
				if (iChomperFrameDP == 14) { iChomperFrameDP = 1; }
				iSwordFrameDP++;
				if (iSwordFrameDP == 49) { iSwordFrameDP = 1; }
				ShowScreen (iScreen);
				oldticks = newticks;
			}
		}

		while (SDL_PollEvent (&event))
		{
			if (MapEvents (event) == 0)
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							InitScreenAction ("enter");
							break;
						case SDL_CONTROLLER_BUTTON_B:
							switch (iScreen)
							{
								case 1:
									Quit(); break;
								case 2:
									iBrokenRoomLinks = BrokenRoomLinks (0);
									/*** no break ***/
								case 3:
									iScreen = 1; break;
							}
							break;
						case SDL_CONTROLLER_BUTTON_X:
							if (iScreen != 2)
							{
								iScreen = 2;
								iMovingRoom = 0;
								iMovingNewBusy = 0;
								iChangingBrokenRoom = iCurRoom;
								iChangingBrokenSide = 1;
								PlaySound ("wav/screen2or3.wav");
							} else if (iBrokenRoomLinks == 0) {
								iBrokenRoomLinks = 1;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDL_CONTROLLER_BUTTON_Y:
							if (iScreen == 2)
							{
								iBrokenRoomLinks = BrokenRoomLinks (0); /*** Why? ***/
							}
							if (iScreen != 3)
							{
								iScreen = 3;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDL_CONTROLLER_BUTTON_BACK:
							if (iScreen == 2)
							{
								if (iBrokenRoomLinks == 1)
								{
									LinkMinus();
								} else {
									MapShow();
								}
							}
							if (iScreen == 3)
							{
								if (EventInfo (iChangeEvent, 3) == 0) /*** if "n" ***/
									{ EventNext (0); } /*** "y" ***/
										else { EventNext (1); } /*** "n" ***/
							}
							break;
						case SDL_CONTROLLER_BUTTON_GUIDE:
							SaveLevel (iCurLevel); break;
						case SDL_CONTROLLER_BUTTON_START:
							RunLevel (iCurLevel); break;
						case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
							if ((iChanged != 0) && (iCurLevel != 1)) { InitPopUpSave(); }
							Prev(); break;
						case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
							if ((iChanged != 0) && (iCurLevel != 0)) { InitPopUpSave(); }
							Next(); break;
						case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							InitScreenAction ("left"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							InitScreenAction ("right"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_UP:
							InitScreenAction ("up"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							InitScreenAction ("down"); break;
					}
					ShowScreen (iScreen);
					break;
				case SDL_CONTROLLERAXISMOTION: /*** triggers and analog sticks ***/
					iXJoy1 = SDL_JoystickGetAxis (joystick, 0);
					iYJoy1 = SDL_JoystickGetAxis (joystick, 1);
					iXJoy2 = SDL_JoystickGetAxis (joystick, 3);
					iYJoy2 = SDL_JoystickGetAxis (joystick, 4);
					if ((iXJoy1 < -30000) || (iXJoy2 < -30000)) /*** left ***/
					{
						if ((SDL_GetTicks() - joyleft) > 300)
						{
							switch (iScreen)
							{
								case 1:
									if (iRoomConnections[iCurRoom][1] != 0)
									{
										iCurRoom = iRoomConnections[iCurRoom][1];
										PlaySound ("wav/scroll.wav");
									}
									break;
								case 3:
									ChangeEvent (-1, 0);
									break;
							}
							joyleft = SDL_GetTicks();
						}
					}
					if ((iXJoy1 > 30000) || (iXJoy2 > 30000)) /*** right ***/
					{
						if ((SDL_GetTicks() - joyright) > 300)
						{
							switch (iScreen)
							{
								case 1:
									if (iRoomConnections[iCurRoom][2] != 0)
									{
										iCurRoom = iRoomConnections[iCurRoom][2];
										PlaySound ("wav/scroll.wav");
									}
									break;
								case 3:
									ChangeEvent (1, 0);
									break;
							}
							joyright = SDL_GetTicks();
						}
					}
					if ((iYJoy1 < -30000) || (iYJoy2 < -30000)) /*** up ***/
					{
						if ((SDL_GetTicks() - joyup) > 300)
						{
							switch (iScreen)
							{
								case 1:
									if (iRoomConnections[iCurRoom][3] != 0)
									{
										iCurRoom = iRoomConnections[iCurRoom][3];
										PlaySound ("wav/scroll.wav");
									}
									break;
								case 3:
									ChangeEvent (10, 0);
									break;
							}
							joyup = SDL_GetTicks();
						}
					}
					if ((iYJoy1 > 30000) || (iYJoy2 > 30000)) /*** down ***/
					{
						if ((SDL_GetTicks() - joydown) > 300)
						{
							switch (iScreen)
							{
								case 1:
									if (iRoomConnections[iCurRoom][4] != 0)
									{
										iCurRoom = iRoomConnections[iCurRoom][4];
										PlaySound ("wav/scroll.wav");
									}
									break;
								case 3:
									ChangeEvent (-10, 0);
									break;
							}
							joydown = SDL_GetTicks();
						}
					}
					if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
					{
						if ((SDL_GetTicks() - trigleft) > 300)
						{
							InitScreenAction ("left bracket");
							trigleft = SDL_GetTicks();
						}
					}
					if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
					{
						if ((SDL_GetTicks() - trigright) > 300)
						{
							InitScreenAction ("right bracket");
							trigright = SDL_GetTicks();
						}
					}
					ShowScreen (iScreen);
					break;
				case SDL_KEYDOWN: /*** http://wiki.libsdl.org/SDL_Keycode ***/
					switch (event.key.keysym.sym)
					{
						case SDLK_F1: if (iScreen == 1) { Help(); } break;
						case SDLK_F2: if (iScreen == 1) { EXE(); } break;
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
							if (((event.key.keysym.mod & KMOD_LALT) ||
								(event.key.keysym.mod & KMOD_RALT)) && (iScreen == 1))
							{
								Zoom (1);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							} else {
								InitScreenAction ("enter");
							}
							break;
						case SDLK_LEFT:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								switch (iScreen)
								{
									case 1:
										if (iRoomConnections[iCurRoom][1] != 0)
										{
											iCurRoom = iRoomConnections[iCurRoom][1];
											PlaySound ("wav/scroll.wav");
										}
										break;
									case 3:
										ChangeEvent (-1, 0);
										break;
								}
							} else if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								if (iScreen == 3) { ChangeEvent (-10, 0); }
							} else {
								InitScreenAction ("left");
							}
							break;
						case SDLK_RIGHT:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								switch (iScreen)
								{
									case 1:
										if (iRoomConnections[iCurRoom][2] != 0)
										{
											iCurRoom = iRoomConnections[iCurRoom][2];
											PlaySound ("wav/scroll.wav");
										}
										break;
									case 3:
										ChangeEvent (1, 0);
										break;
								}
							} else if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								if (iScreen == 3) { ChangeEvent (10, 0); }
							} else {
								InitScreenAction ("right");
							}
							break;
						case SDLK_UP:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								if (iScreen == 1)
								{
									if (iRoomConnections[iCurRoom][3] != 0)
									{
										iCurRoom = iRoomConnections[iCurRoom][3];
										PlaySound ("wav/scroll.wav");
									}
								}
							} else {
								InitScreenAction ("up");
							}
							break;
						case SDLK_DOWN:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								if (iScreen == 1)
								{
									if (iRoomConnections[iCurRoom][4] != 0)
									{
										iCurRoom = iRoomConnections[iCurRoom][4];
										PlaySound ("wav/scroll.wav");
									}
								}
							} else {
								InitScreenAction ("down");
							}
							break;
						case SDLK_LEFTBRACKET:
							InitScreenAction ("left bracket");
							break;
						case SDLK_RIGHTBRACKET:
							InitScreenAction ("right bracket");
							break;
						case SDLK_MINUS:
						case SDLK_KP_MINUS:
							if ((iChanged != 0) && (iCurLevel != 1)) { InitPopUpSave(); }
							Prev(); break;
						case SDLK_KP_PLUS:
						case SDLK_EQUALS:
							if ((iChanged != 0) && (iCurLevel != 0)) { InitPopUpSave(); }
							Next(); break;
						case SDLK_QUOTE:
							if (iScreen == 1)
							{
								if ((event.key.keysym.mod & KMOD_LSHIFT) ||
									(event.key.keysym.mod & KMOD_RSHIFT))
								{
									Sprinkle();
									PlaySound ("wav/extras.wav");
									iChanged++;
								} else {
									SetLocation (iCurRoom, iSelected, iLastThing, iLastModifier);
									PlaySound ("wav/ok_close.wav");
									iChanged++;
								}
							}
							break;
						case SDLK_d: RunLevel (iCurLevel); break;
						case SDLK_s:
							SaveLevel (iCurLevel);
							break;
						case SDLK_t:
							if (iScreen == 1) { InitScreenAction ("env"); }
							break;
						case SDLK_i:
							if (iScreen == 1)
							{
								if (iInfo == 0) { iInfo = 1; } else { iInfo = 0; }
							}
							break;
						case SDLK_r:
							if (iScreen != 2)
							{
								iScreen = 2;
								iMovingRoom = 0;
								iMovingNewBusy = 0;
								iChangingBrokenRoom = iCurRoom;
								iChangingBrokenSide = 1;
								PlaySound ("wav/screen2or3.wav");
							} else if (iBrokenRoomLinks == 0) {
								iBrokenRoomLinks = 1;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDLK_e:
							if (iScreen == 2)
							{
								iBrokenRoomLinks = BrokenRoomLinks (0); /*** Why? ***/
							}
							if (iScreen != 3)
							{
								iScreen = 3;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDLK_c:
							if (iScreen == 1)
							{
								if ((event.key.keysym.mod & KMOD_LCTRL) ||
									(event.key.keysym.mod & KMOD_RCTRL))
								{
									CopyPaste (iCurRoom, 1);
									PlaySound ("wav/extras.wav");
								}
							}
							break;
						case SDLK_z:
							if (iScreen == 1)
							{
								Zoom (0);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							}
							break;
						case SDLK_f:
							if (iScreen == 1)
							{
								Zoom (1);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							}
							break;
						case SDLK_h:
							if (iScreen == 1)
							{
								FlipRoom (iCurRoom, 1);
								PlaySound ("wav/extras.wav");
								iChanged++;
							}
							break;
						case SDLK_v:
							if (iScreen == 1)
							{
								if ((event.key.keysym.mod & KMOD_LCTRL) ||
									(event.key.keysym.mod & KMOD_RCTRL))
								{
									CopyPaste (iCurRoom, 2);
									PlaySound ("wav/extras.wav");
									iChanged++;
								} else {
									FlipRoom (iCurRoom, 2);
									PlaySound ("wav/extras.wav");
									iChanged++;
								}
							}
							break;
						case SDLK_y:
							if (iScreen == 3)
							{
								if (EventInfo (iChangeEvent, 3) == 0) { EventNext (0); }
							}
							break;
						case SDLK_n:
							if (iScreen == 3)
							{
								if (EventInfo (iChangeEvent, 3) == 1) { EventNext (1); }
							}
							break;
						case SDLK_m:
							if (iBrokenRoomLinks == 0)
							{
								if (iMapOpen == 0)
								{
									MapShow();
								} else {
									SDL_RaiseWindow (windowmap);
								}
							}
							break;
						case SDLK_BACKSPACE:
							if ((iScreen == 2) && (iBrokenRoomLinks == 1))
								{ LinkMinus(); }
							break;
						case SDLK_ESCAPE:
						case SDLK_q:
							switch (iScreen)
							{
								case 1:
									Quit(); break;
								case 2:
									iBrokenRoomLinks = BrokenRoomLinks (0);
									/*** no break ***/
								case 3:
									iScreen = 1; break;
							}
							break;
						case SDLK_BACKSLASH:
							if (iScreen == 1)
							{
								/*** Randomize the entire level. ***/
								for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
								{
									for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
									{
										UseTile (-1, iLoopTile, iLoopRoom);
									}
								}
								PlaySound ("wav/ok_close.wav");
								iChanged++;
							}
							break;
						case SDLK_SLASH:
							if (iScreen == 1) { ClearRoom(); }
							break;
						case SDLK_0: /*** empty ***/
						case SDLK_KP_0:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x00, 0x00);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_1: /*** floor ***/
						case SDLK_KP_1:
							if (iScreen == 1)
							{
								if (cCurType == 'd')
								{
									SetLocation (iCurRoom, iSelected, 0x01, 0x00);
								} else {
									SetLocation (iCurRoom, iSelected, 0x01, 0x01);
								}
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_2: /*** loose tile ***/
						case SDLK_KP_2:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x0B, 0x00);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_3: /*** closed gate ***/
						case SDLK_KP_3:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x04, 0x00);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_4: /*** open gate ***/
						case SDLK_KP_4:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x04, 0x01);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_5: /*** flame ***/
						case SDLK_KP_5:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x13, 0x00);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_6: /*** spike ***/
						case SDLK_KP_6:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x02, 0x00);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_7: /*** small pillar ***/
						case SDLK_KP_7:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x03, 0x00);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_8: /*** chomper ***/
						case SDLK_KP_8:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x12, 0x00);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_9: /*** wall ***/
						case SDLK_KP_9:
							if (iScreen == 1)
							{
								SetLocation (iCurRoom, iSelected, 0x14, 0x01);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
					}
					ShowScreen (iScreen);
					break;
				case SDL_MOUSEMOTION:
					iXPosOld = iXPos;
					iYPosOld = iYPos;
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					if ((iXPosOld == iXPos) && (iYPosOld == iYPos)) { break; }

					if (OnLevelBar() == 1)
						{ iPlaytest = 1; } else { iPlaytest = 0; }

					if (iScreen == 1)
					{
						/*** hover ***/
						for (iLoopTile = 1; iLoopTile <= 30; iLoopTile++)
						{
							if ((iLoopTile >= 1) && (iLoopTile <= 10))
								{ iLocX = iLoopTile; iLocY = 1; }
							if ((iLoopTile >= 11) && (iLoopTile <= 20))
								{ iLocX = iLoopTile - 10; iLocY = 2; }
							if ((iLoopTile >= 21) && (iLoopTile <= 30))
								{ iLocX = iLoopTile - 20; iLocY = 3; }

							if ((InArea (25 + (64 * (iLocX - 1)), 56 + (126 * (iLocY - 1)),
								25 + (64 * iLocX), 56 + (126 * iLocY)) == 1) &&
								(iSelected != iLoopTile)) { iSelected = iLoopTile; }
						}

						/*** extras ***/
						if (InArea (608, 3, 608 + 49, 3 + 19) == 0) { iExtras = 0; }
						if (InArea (608, 3, 608 + 9, 3 + 9) == 1) { iExtras = 1; }
						if (InArea (618, 3, 618 + 9, 3 + 9) == 1) { iExtras = 2; }
						if (InArea (628, 3, 628 + 9, 3 + 9) == 1) { iExtras = 3; }
						if (InArea (638, 3, 638 + 9, 3 + 9) == 1) { iExtras = 4; }
						if (InArea (648, 3, 648 + 9, 3 + 9) == 1) { iExtras = 5; }
						if (InArea (608, 13, 608 + 9, 13 + 9) == 1) { iExtras = 6; }
						if (InArea (618, 13, 618 + 9, 13 + 9) == 1) { iExtras = 7; }
						if (InArea (628, 13, 628 + 9, 13 + 9) == 1) { iExtras = 8; }
						if (InArea (638, 13, 638 + 9, 13 + 9) == 1) { iExtras = 9; }
						if (InArea (648, 13, 648 + 9, 13 + 9) == 1) { iExtras = 10; }
					}

					if (iScreen == 2)
					{
						/*** if (iMovingRoom != 0) { ShowScreen (2); } ***/
					}

					if (iScreen == 3)
					{
						/* A tiny Easter egg: the mouse looks up if the user
						 * hovers over it.
						 */
						if (InArea (608, 71, 608 + 46, 71 + 21) == 1)
						{
							if (iMouse == 0) { PlaySound ("wav/squeak.wav"); }
							iMouse = 1;
						} else { iMouse = 0; }
					}

					ShowScreen (iScreen);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (0, 50, 0 + 25, 50 + 384) == 1) /*** left ***/
						{
							if (iRoomConnections[iCurRoom][1] != 0) { iDownAt = 1; }
						}
						if (InArea (665, 50, 665 + 25, 50 + 384) == 1) /*** right ***/
						{
							if (iRoomConnections[iCurRoom][2] != 0) { iDownAt = 2; }
						}
						if (InArea (25, 25, 25 + 640, 25 + 25) == 1) /*** up ***/
						{
							if (iRoomConnections[iCurRoom][3] != 0) { iDownAt = 3; }
						}
						if (InArea (25, 434, 25 + 640, 434 + 25) == 1) /*** down ***/
						{
							if (iRoomConnections[iCurRoom][4] != 0) { iDownAt = 4; }
						}
						if (InArea (0, 25, 0 + 25, 25 + 25) == 1) /*** rooms ***/
						{
							if (iBrokenRoomLinks == 0)
								{ iDownAt = 5; } else { iDownAt = 11; }
						}
						if (InArea (665, 25, 665 + 25, 25 + 25) == 1) /*** events ***/
							{ iDownAt = 6; }
						if (InArea (0, 434, 0 + 25, 434 + 25) == 1) /*** save ***/
							{ iDownAt = 7; }
						if (InArea (665, 434, 665 + 25, 434 + 25) == 1) /*** quit ***/
							{ iDownAt = 8; }
						if (InArea (0, 0, 0 + 25, 0 + 25) == 1) /*** prev level ***/
							{ iDownAt = 9; }
						if (InArea (665, 0, 665 + 25, 0 + 25) == 1) /*** next level ***/
							{ iDownAt = 10; }

						if (iScreen == 2)
						{
							if (iBrokenRoomLinks == 0)
							{
								for (iLoopX = 1; iLoopX <= ROOMS; iLoopX++)
								{
									for (iLoopY = 1; iLoopY <= ROOMS; iLoopY++)
									{
										if (InArea (294 + ((iLoopX - 1) * 15),
											63 + ((iLoopY - 1) * 15),
											294 + ((iLoopX - 1) * 15) + 14,
											63 + ((iLoopY - 1) * 15) + 14) == 1)
										{
											if (iRoomArray[iLoopX][iLoopY] != 0)
											{
												iMovingNewBusy = 0;
												iMovingRoom = iRoomArray[iLoopX][iLoopY];
											}
										}
									}
								}

								/*** side pane ***/
								for (iLoopY = 1; iLoopY <= ROOMS; iLoopY++)
								{
									if (InArea (271, 63 + ((iLoopY - 1) * 15),
										271 + 14, 63 + ((iLoopY - 1) * 15) + 14) == 1)
									{
										if (iRoomArray[25][iLoopY] != 0)
										{
											iMovingNewBusy = 0;
											iMovingRoom = iRoomArray[25][iLoopY];
										}
									}
								}

								if (InArea (596, 65, 596 + 25, 65 + 25) == 1)
									{ iDownAt = 12; } /*** Map window ***/
								if (InArea (626, 65, 626 + 25, 65 + 25) == 1)
									{ iDownAt = 11; } /*** rooms broken ***/
							} else {
								MouseSelectAdj();
							}
						}
					}
					ShowScreen (iScreen);
					break;
				case SDL_MOUSEBUTTONUP:
					iDownAt = 0;
					if (event.button.button == 1)
					{
						if (InArea (0, 50, 0 + 25, 50 + 384) == 1) /*** left ***/
						{
							if (iRoomConnections[iCurRoom][1] != 0)
							{
								iCurRoom = iRoomConnections[iCurRoom][1];
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (665, 50, 665 + 25, 50 + 384) == 1) /*** right ***/
						{
							if (iRoomConnections[iCurRoom][2] != 0)
							{
								iCurRoom = iRoomConnections[iCurRoom][2];
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (25, 25, 25 + 640, 25 + 25) == 1) /*** up ***/
						{
							if (iRoomConnections[iCurRoom][3] != 0)
							{
								iCurRoom = iRoomConnections[iCurRoom][3];
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (25, 434, 25 + 640, 434 + 25) == 1) /*** down ***/
						{
							if (iRoomConnections[iCurRoom][4] != 0)
							{
								iCurRoom = iRoomConnections[iCurRoom][4];
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (0, 25, 0 + 25, 25 + 25) == 1) /*** rooms ***/
						{
							if (iScreen != 2)
							{
								iScreen = 2; iMovingRoom = 0; iMovingNewBusy = 0;
								iChangingBrokenRoom = iCurRoom;
								iChangingBrokenSide = 1;
								PlaySound ("wav/screen2or3.wav");
							}
						}
						if (InArea (665, 25, 665 + 25, 25 + 25) == 1) /*** events ***/
						{
							if (iScreen == 2)
							{
								iBrokenRoomLinks = BrokenRoomLinks (0);
							}
							if (iScreen != 3)
							{
								iScreen = 3;
								PlaySound ("wav/screen2or3.wav");
							}
						}
						if (InArea (0, 434, 0 + 25, 434 + 25) == 1) /*** save ***/
						{
							SaveLevel (iCurLevel);
						}
						if (InArea (665, 434, 665 + 25, 434 + 25) == 1) /*** quit ***/
						{
							switch (iScreen)
							{
								case 1: Quit(); break;
								case 2:
									iBrokenRoomLinks = BrokenRoomLinks (0);
									/*** no break ***/
								case 3: iScreen = 1; break;
							}
						}
						if (InArea (0, 0, 0 + 25, 0 + 25) == 1) /*** prev level ***/
						{
							if ((iChanged != 0) && (iCurLevel != 1)) { InitPopUpSave(); }
							Prev();
							break; /*** Stop processing SDL_MOUSEBUTTONUP. ***/
						}
						if (InArea (665, 0, 665 + 25, 0 + 25) == 1) /*** next level ***/
						{
							if ((iChanged != 0) && (iCurLevel != 0)) { InitPopUpSave(); }
							Next();
							break; /*** Stop processing SDL_MOUSEBUTTONUP. ***/
						}
						if (OnLevelBar() == 1)
						{
							RunLevel (iCurLevel);
							break; /*** Stop processing SDL_MOUSEBUTTONUP. ***/
						}

						if (iScreen == 1)
						{
							if (InArea (25, 50, 25 + 640, 50 + 384) == 1) /*** tiles ***/
							{
								keystate = SDL_GetKeyboardState (NULL);
								if ((keystate[SDL_SCANCODE_LSHIFT]) ||
									(keystate[SDL_SCANCODE_RSHIFT]))
								{
									SetLocation (iCurRoom, iSelected, iLastThing, iLastModifier);
									PlaySound ("wav/ok_close.wav");
									iChanged++;
								} else {
									ChangePos (iSelected);
								}
							}

							/*** 1 ***/
							if (InArea (608, 3, 608 + 9, 3 + 9) == 1)
							{
								Zoom (0);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							}

							/*** 2 ***/
							if (InArea (618, 3, 618 + 9, 3 + 9) == 1)
							{
								CopyPaste (iCurRoom, 1);
								PlaySound ("wav/extras.wav");
							}

							/*** 3 ***/
							if (InArea (628, 3, 628 + 9, 3 + 9) == 1)
							{
								FlipRoom (iCurRoom, 2);
								PlaySound ("wav/extras.wav");
								iChanged++;
							}

							/*** 4 ***/
							if (InArea (638, 3, 638 + 9, 3 + 9) == 1)
								{ InitScreenAction ("env"); }

							/*** 5 ***/
							if (InArea (648, 3, 648 + 9, 3 + 9) == 1) { Help(); }

							/*** 6 ***/
							if (InArea (608, 13, 608 + 9, 13 + 9) == 1)
							{
								Sprinkle();
								PlaySound ("wav/extras.wav");
								iChanged++;
							}

							/*** 7 ***/
							if (InArea (618, 13, 618 + 9, 13 + 9) == 1)
							{
								CopyPaste (iCurRoom, 2);
								PlaySound ("wav/extras.wav");
								iChanged++;
							}

							/*** 8 ***/
							if (InArea (628, 13, 628 + 9, 13 + 9) == 1)
							{
								FlipRoom (iCurRoom, 1);
								PlaySound ("wav/extras.wav");
								iChanged++;
							}

							/*** 9 ***/
							if (InArea (638, 13, 638 + 9, 13 + 9) == 1)
								{ /*** Undo. ***/ }

							/*** 10 ***/
							if (InArea (648, 13, 648 + 9, 13 + 9) == 1) { EXE(); }
						}

						if (iScreen == 2)
						{
							if (iBrokenRoomLinks == 0)
							{
								for (iLoopX = 1; iLoopX <= ROOMS; iLoopX++)
								{
									for (iLoopY = 1; iLoopY <= ROOMS; iLoopY++)
									{
										if (InArea (294 + ((iLoopX - 1) * 15),
											63 + ((iLoopY - 1) * 15),
											294 + ((iLoopX - 1) * 15) + 14,
											63 + ((iLoopY - 1) * 15) + 14) == 1)
										{
											if (iMovingRoom != 0)
											{
												if (iRoomArray[iLoopX][iLoopY] == 0)
												{
													RemoveOldRoom();
													AddNewRoom (iLoopX, iLoopY, iMovingRoom);
													iChanged++;
												}
												iMovingRoom = 0; iMovingNewBusy = 0;
											}
										}
									}
								}

								/*** side pane ***/
								for (iLoopY = 1; iLoopY <= ROOMS; iLoopY++)
								{
									if (InArea (271, 63 + ((iLoopY - 1) * 15),
										271 + 14, 63 + ((iLoopY - 1) * 15) + 14) == 1)
									{
										if (iMovingRoom != 0)
										{
											if (iRoomArray[25][iLoopY] == 0)
											{
												RemoveOldRoom();
												AddNewRoom (25, iLoopY, iMovingRoom);
												iChanged++;
											}
											iMovingRoom = 0; iMovingNewBusy = 0;
										}
									}
								}

								if (InArea (596, 65, 596 + 25, 65 + 25) == 1)
									{ MapShow(); } /*** Map window ***/
								if (InArea (626, 65, 626 + 25, 65 + 25) == 1)
								{ /*** rooms broken ***/
									iBrokenRoomLinks = 1;
									PlaySound ("wav/screen2or3.wav");
								}
							} else {
								if (MouseSelectAdj() == 1) { LinkPlus(); }
							}
						}

						if (iScreen == 3)
						{
							/*** event number ***/
							if (InArea (340, 62, 340 + 13, 62 + 20) == 1)
								{ ChangeEvent (-10, 0); }
							if (InArea (355, 62, 355 + 13, 62 + 20) == 1)
								{ ChangeEvent (-1, 0); }
							if (InArea (425, 62, 425 + 13, 62 + 20) == 1)
								{ ChangeEvent (1, 0); }
							if (InArea (440, 62, 440 + 13, 62 + 20) == 1)
								{ ChangeEvent (10, 0); }

							/*** room ***/
							for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
							{
								if (InArea (279 + (15 * iLoopRoom), 117,
									279 + (15 * iLoopRoom) + 14, 117 + 14) == 1)
									{ EventRoom (iLoopRoom); }
							}

							/*** tile ***/
							for (iLoopTile = 1; iLoopTile <= 10; iLoopTile++)
							{
								if (InArea (444 + (15 * iLoopTile), 157,
									444 + (15 * iLoopTile) + 14, 157 + 14) == 1)
									{ EventDoor (iLoopTile); }
							}
							for (iLoopTile = 11; iLoopTile <= 20; iLoopTile++)
							{
								if (InArea (444 + (15 * (iLoopTile - 10)), 172,
									444 + (15 * (iLoopTile - 10)) + 14, 172 + 14) == 1)
									{ EventDoor (iLoopTile); }
							}
							for (iLoopTile = 21; iLoopTile <= 30; iLoopTile++)
							{
								if (InArea (444 + (15 * (iLoopTile - 20)), 187,
									444 + (15 * (iLoopTile - 20)) + 14, 187 + 14) == 1)
									{ EventDoor (iLoopTile); }
							}

							/*** next ***/
							if (InArea (579, 227, 579 + 14, 227 + 14) == 1)
								{ EventNext (1); } /*** n ***/
							if (InArea (594, 227, 594 + 14, 227 + 14) == 1)
								{ EventNext (0); } /*** y ***/
						}
					}
					if (event.button.button == 2)
					{
						if (iScreen == 1) { ClearRoom(); }
					}
					if (event.button.button == 3)
					{
						if (iScreen == 1)
						{
							/*** Randomize the entire level. ***/
							for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
							{
								for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
								{
									UseTile (-1, iLoopTile, iLoopRoom);
								}
							}
							PlaySound ("wav/ok_close.wav");
							iChanged++;
						}
						if (iScreen == 2)
						{
							if (iBrokenRoomLinks == 1)
							{
								if (MouseSelectAdj() == 1) { LinkMinus(); }
							}
						}
					}
					ShowScreen (iScreen);
					break;
				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0) /*** scroll wheel up ***/
					{
						if (InArea (25, 50, 25 + 640, 50 + 384) == 1) /*** tiles ***/
						{
							keystate = SDL_GetKeyboardState (NULL);
							if ((keystate[SDL_SCANCODE_LSHIFT]) ||
								(keystate[SDL_SCANCODE_RSHIFT]))
							{ /*** right ***/
								if (iRoomConnections[iCurRoom][2] != 0)
								{
									iCurRoom = iRoomConnections[iCurRoom][2];
									PlaySound ("wav/scroll.wav");
								}
							} else { /*** up ***/
								if (iRoomConnections[iCurRoom][3] != 0)
								{
									iCurRoom = iRoomConnections[iCurRoom][3];
									PlaySound ("wav/scroll.wav");
								}
							}
						}
					}
					if (event.wheel.y < 0) /*** scroll wheel down ***/
					{
						if (InArea (25, 50, 25 + 640, 50 + 384) == 1) /*** tiles ***/
						{
							keystate = SDL_GetKeyboardState (NULL);
							if ((keystate[SDL_SCANCODE_LSHIFT]) ||
								(keystate[SDL_SCANCODE_RSHIFT]))
							{ /*** left ***/
								if (iRoomConnections[iCurRoom][1] != 0)
								{
									iCurRoom = iRoomConnections[iCurRoom][1];
									PlaySound ("wav/scroll.wav");
								}
							} else { /*** down ***/
								if (iRoomConnections[iCurRoom][4] != 0)
								{
									iCurRoom = iRoomConnections[iCurRoom][4];
									PlaySound ("wav/scroll.wav");
								}
							}
						}
					}
					ShowScreen (iScreen);
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED:
							ShowScreen (iScreen); break;
						case SDL_WINDOWEVENT_CLOSE:
							Quit(); break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							iActiveWindowID = iWindowID; break;
					}
					break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
}
/*****************************************************************************/
void LoadFonts (void)
/*****************************************************************************/
{
	font11 = TTF_OpenFont ("ttf/Bitstream-Vera-Sans-Bold.ttf", 11 * iScale);
	font15 = TTF_OpenFont ("ttf/Bitstream-Vera-Sans-Bold.ttf", 15 * iScale);
	font20 = TTF_OpenFont ("ttf/Bitstream-Vera-Sans-Bold.ttf", 20 * iScale);
	if ((font11 == NULL) || (font15 == NULL) || (font20 == NULL))
	{
		printf ("[FAILED] TTF_OpenFont() failed!\n");
		exit (EXIT_ERROR);
	}
}
/*****************************************************************************/
void MixAudio (void *unused, Uint8 *stream, int iLen)
/*****************************************************************************/
{
	int iTemp;
	int iAmount;

	if (unused != NULL) { } /*** To prevent warnings. ***/

	SDL_memset (stream, 0, iLen); /*** SDL2 ***/
	for (iTemp = 0; iTemp < NUM_SOUNDS; iTemp++)
	{
		iAmount = (sounds[iTemp].dlen-sounds[iTemp].dpos);
		if (iAmount > iLen)
		{
			iAmount = iLen;
		}
		SDL_MixAudio (stream, &sounds[iTemp].data[sounds[iTemp].dpos], iAmount,
			SDL_MIX_MAXVOLUME);
		sounds[iTemp].dpos += iAmount;
	}
}
/*****************************************************************************/
void PreLoad (char *sPath, char *sPNG, SDL_Texture **imgImage)
/*****************************************************************************/
{
	char sImage[MAX_IMG + 2];
	int iBarHeight;

	snprintf (sImage, MAX_IMG, "%s%s", sPath, sPNG);
	*imgImage = IMG_LoadTexture (ascreen, sImage);
	if (!*imgImage)
	{
		printf ("[FAILED] IMG_LoadTexture: %s!\n", IMG_GetError());
		exit (EXIT_ERROR);
	}

	iPreLoaded++;
	iBarHeight = (int)(((float)iPreLoaded/(float)iNrToPreLoad) * BAR_FULL);
	if (iBarHeight >= iCurrentBarHeight + 10) { LoadingBar (iBarHeight); }
}
/*****************************************************************************/
void PreLoadMap (char *sPath, char *sPNG, SDL_Texture **imgImage)
/*****************************************************************************/
{
	char sImage[MAX_IMG + 2];
	int iBarHeight;

	snprintf (sImage, MAX_IMG, "%s%s", sPath, sPNG);
	*imgImage = IMG_LoadTexture (mscreen, sImage);
	if (!*imgImage)
	{
		printf ("[FAILED] IMG_LoadTexture: %s\n", IMG_GetError());
		exit (EXIT_ERROR);
	}

	iPreLoaded++;
	iBarHeight = (int)(((float)iPreLoaded/(float)iNrToPreLoad) * BAR_FULL);
	if (iBarHeight >= iCurrentBarHeight + 10) { LoadingBar (iBarHeight); }
}
/*****************************************************************************/
void ShowImage (SDL_Texture *img, int iX, int iY, char *sImageInfo,
	SDL_Renderer *screen, float fMultiply, int iXYScale)
/*****************************************************************************/
{
	/* Usually, fMultiply is either iScale or ZoomGet().
	 *
	 * Also, usually, iXYScale is 1 with iScale and 0 with ZoomGet().
	 *
	 * If you use this function, make sure to verify the image is properly
	 * (re)scaled and (re)positioned when the main window is zoomed ("z")!
	 */

	SDL_Rect dest;
	SDL_Rect loc;
	int iWidth, iHeight;

	SDL_QueryTexture (img, NULL, NULL, &iWidth, &iHeight);
	loc.x = 0; loc.y = 0; loc.w = iWidth; loc.h = iHeight;
	if (iXYScale == 0)
	{
		dest.x = iX;
		dest.y = iY;
	} else {
		dest.x = iX * fMultiply;
		dest.y = iY * fMultiply;
	}
	dest.w = iWidth * fMultiply;
	dest.h = iHeight * fMultiply;

	/*** This is for the animation. ***/
	if (strcmp (sImageInfo, "imghourglasssprite") == 0)
	{
		loc.x = (iHourglassFrame - 1) * 38;
		loc.w = loc.w / 7;
		dest.w = dest.w / 7;
	}
	if (strcmp (sImageInfo, "imgsandsprite") == 0)
	{
		loc.x = (iSandFrame - 1) * 5;
		loc.w = loc.w / 3;
		dest.w = dest.w / 3;
	}
	if (strcmp (sImageInfo, "imgstatusbarsprite") == 0)
	{
		loc.x = (iStatusBarFrame - 1) * 20;
		loc.w = loc.w / 18;
		dest.w = dest.w / 18;
	}
	if ((strcmp (sImageInfo, "spriteflamed1") == 0) ||
		(strcmp (sImageInfo, "spriteflamed2") == 0) ||
		(strcmp (sImageInfo, "spriteflamep1") == 0) ||
		(strcmp (sImageInfo, "spriteflamep2") == 0))
	{
		loc.x = (iFlameFrameDP - 1) * 129;
		loc.w = loc.w / 9;
		dest.w = dest.w / 9;
	}
	if ((strcmp (sImageInfo, "spritechomperd1") == 0) ||
		(strcmp (sImageInfo, "spritechomperd2") == 0) ||
		(strcmp (sImageInfo, "spritechomperp1") == 0) ||
		(strcmp (sImageInfo, "spritechomperp2") == 0) ||
		(strcmp (sImageInfo, "spritechompersel1") == 0) ||
		(strcmp (sImageInfo, "spritechompersel2") == 0))
	{
		switch (iChomperFrameDP)
		{
			case 10: loc.x = 129; break;
			case 11: loc.x = 2 * 129; break;
			case 12: loc.x = 3 * 129; break;
			case 13: loc.x = 4 * 129; break;
			default: loc.x = 0; break; /*** 1-9 ***/
		}
		loc.w = loc.w / 5;
		dest.w = dest.w / 5;
	}
	if ((strcmp (sImageInfo, "spriteswordd") == 0) ||
		(strcmp (sImageInfo, "spriteswordp") == 0) ||
		(strcmp (sImageInfo, "spriteswordsel") == 0))
	{
		switch (iSwordFrameDP)
		{
			case 48: loc.x = 129; break;
			default: loc.x = 0; break; /*** 1-47 ***/
		}
		loc.w = loc.w / 2;
		dest.w = dest.w / 2;
	}

	if (SDL_RenderCopy (screen, img, &loc, &dest) != 0)
	{
		printf ("[ WARN ] SDL_RenderCopy (%s): %s\n", sImageInfo, SDL_GetError());
	}
}
/*****************************************************************************/
void LoadingBar (int iBarHeight)
/*****************************************************************************/
{
	SDL_Rect bar;

	/*** bar ***/
	bar.x = 12 * iScale;
	bar.y = (447 - iBarHeight) * iScale;
	bar.w = 16 * iScale;
	bar.h = iBarHeight * iScale;
	SDL_SetRenderDrawColor (ascreen, 0x44, 0x44, 0x44, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect (ascreen, &bar);
	iCurrentBarHeight = iBarHeight;

	/*** hourglass sprite ***/
	iHourglassFrame = floor (((float)iBarHeight/BAR_FULL) * 7);
	ShowImage (imghourglasssprite, 326, 174, "imghourglasssprite",
		ascreen, iScale, 1);

	/*** sand sprite ***/
	if (iBarHeight != BAR_FULL)
	{
		ShowImage (imgsandsprite, 342, 202, "imgsandsprite",
		ascreen, iScale, 1);
		iSandFrame++;
		if (iSandFrame == 4) { iSandFrame = 1; }
	}

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void ShowScreen (int iScreenS)
/*****************************************************************************/
{
	int iLoc;
	char sLevel[MAX_TEXT + 2];
	char arText[1 + 2][MAX_TEXT + 2];
	int iImgDir, iImgX, iImgY;
	int iUnusedRooms;
	int iXShow, iYShow;
	char sShowRoom[MAX_TEXT + 2];
	int iEventUnused;
	int iERoom, iETile, iENext;
	int iShowX, iShowY;
	int iXY[2 + 2];

	/*** Used for looping. ***/
	int iLoopTile;
	int iLoopRoom;
	int iLoopSide;

	/*** black background ***/
	ShowImage (imgblack, 0, 0, "imgblack", ascreen, iScale, 1);

	if (iScreenS == 1)
	{
		/*** lower left outside ***/
		if (iRoomConnections[iCurRoom][1] != 0)
		{
			if (iRoomConnections[iRoomConnections[iCurRoom][1]][4] != 0)
			{
				ShowTile (iThingA[iRoomConnections
					[iRoomConnections[iCurRoom][1]][4]][10],
					iModifierA[iRoomConnections
					[iRoomConnections[iCurRoom][1]][4]][10], 103);
			} else { ShowTile (0x14, 0x01, 103); }
		} else { ShowTile (0x14, 0x01, 103); }

		/*** left of this room ***/
		if (iRoomConnections[iCurRoom][1] != 0)
		{
			ShowTile (iThingA[iRoomConnections[iCurRoom][1]][10],
				iModifierA[iRoomConnections[iCurRoom][1]][10], 100);
			ShowTile (iThingA[iRoomConnections[iCurRoom][1]][20],
				iModifierA[iRoomConnections[iCurRoom][1]][20], 101);
			ShowTile (iThingA[iRoomConnections[iCurRoom][1]][30],
				iModifierA[iRoomConnections[iCurRoom][1]][30], 102);
		} else {
			ShowTile (0x14, 0x01, 100);
			ShowTile (0x14, 0x01, 101);
			ShowTile (0x14, 0x01, 102);
		}

		/*** under this room ***/
		if (iRoomConnections[iCurRoom][4] != 0)
		{
			for (iLoopTile = 1; iLoopTile <= 10; iLoopTile++)
			{
				ShowTile (iThingA[iRoomConnections[iCurRoom][4]][iLoopTile],
					iModifierA[iRoomConnections[iCurRoom][4]][iLoopTile],
					TILES + iLoopTile);
			}
		}

		for (iLoopTile = 1; iLoopTile <= 30; iLoopTile++)
		{
			/*** tile ***/
			iLoc = 0; /*** To prevent warnings. ***/
			if ((iLoopTile >= 1) && (iLoopTile <= 10)) { iLoc = 20 + iLoopTile; }
			if ((iLoopTile >= 11) && (iLoopTile <= 20)) { iLoc = iLoopTile; }
			if ((iLoopTile >= 21) && (iLoopTile <= 30)) { iLoc = -20 + iLoopTile; }
			ShowTile (iThingA[iCurRoom][iLoc],
				iModifierA[iCurRoom][iLoc], iLoc);

			LocationToXY (iLoc, iXY);

			/*** prince ***/
			if ((iCurRoom == (int)luKidRoom) && (iLoc == (int)luKidPos))
			{
				switch (luKidDir)
				{
					case 0: /*** looks right ***/
						iImgX = iXY[0] + 74;
						iImgY = iXY[1] + 66;
						ShowImage (imgkidr[1], iImgX, iImgY, "imgkidr[1]",
							ascreen, iScale, 1);
						if (iSelected == iLoc)
						{ ShowImage (imgkidr[2], iImgX, iImgY, "imgkidr[2]",
							ascreen, iScale, 1); }
						break;
					case 255: /*** looks left ***/
						iImgX = iXY[0] + 34;
						iImgY = iXY[1] + 66;
						ShowImage (imgkidl[1], iImgX, iImgY, "imgkidl[1]",
							ascreen, iScale, 1);
						if (iSelected == iLoc)
						{ ShowImage (imgkidl[2], iImgX, iImgY, "imgkidl[2]",
							ascreen, iScale, 1); }
						break;
					default: printf ("[FAILED] Wrong dir.!\n"); exit (EXIT_ERROR); break;
				}
			}

			/*** guard ***/
			if (sGuardLocations[iCurRoom - 1] + 1 == iLoc)
			{
				if (sGuardDirections[iCurRoom - 1] == 255) /*** looks left ***/
				{
					iImgDir = 1;
					switch (iCurGuard)
					{
						case 0: /*** guard ***/
							iImgX = iXY[0] + 44; iImgY = iXY[1] + 66; break;
						case 1: /*** fat ***/
							iImgX = iXY[0] + 44; iImgY = iXY[1] + 67; break;
						case 2: /*** skeleton ***/
							iImgX = iXY[0] + 38; iImgY = iXY[1] + 80; break;
						case 3: /*** Jaffar ***/
							iImgX = iXY[0] + 44; iImgY = iXY[1] + 60; break;
						case 4: /*** shadow ***/
							iImgX = iXY[0] + 38; iImgY = iXY[1] + 77; break;
						default: /*** Fallback. ***/
							iImgX = iXY[0] + 44; iImgY = iXY[1] + 66; break;
					}
				} else { /*** looks right ***/
					iImgDir = 2;
					switch (iCurGuard)
					{
						case 0: /*** guard ***/
							iImgX = iXY[0] + 6; iImgY = iXY[1] + 66; break;
						case 1: /*** fat ***/
							iImgX = iXY[0] + 9; iImgY = iXY[1] + 67; break;
						case 2: /*** skeleton ***/
							iImgX = iXY[0] + 7; iImgY = iXY[1] + 80; break;
						case 3: /*** Jaffar ***/
							iImgX = iXY[0] + 19; iImgY = iXY[1] + 60; break;
						case 4: /*** shadow ***/
							iImgX = iXY[0] + 7; iImgY = iXY[1] + 77; break;
						default: /*** Fallback. ***/
							iImgX = iXY[0] + 6; iImgY = iXY[1] + 66; break;
					}
				}

				switch (iCurGuard)
				{
					case 0: /*** guard ***/
						switch (sGuardColors[iCurRoom - 1])
						{
							case 1: /*** blush ***/
								ShowImage (imggblush[iImgDir], iImgX, iImgY,
									"imggblush[]", ascreen, iScale, 1); break;
							case 2: /*** yellow ***/
								ShowImage (imggyellow[iImgDir], iImgX, iImgY,
									"imggyellow[]", ascreen, iScale, 1); break;
							case 3: /*** rouge ***/
								ShowImage (imggrouge[iImgDir], iImgX, iImgY,
									"imggrouge[]", ascreen, iScale, 1); break;
							case 4: /*** rose ***/
								ShowImage (imggrose[iImgDir], iImgX, iImgY,
									"imggrose[]", ascreen, iScale, 1); break;
							case 5: /*** dgreen ***/
								ShowImage (imggdgreen[iImgDir], iImgX, iImgY,
									"imggdgreen[]", ascreen, iScale, 1); break;
							case 6: /*** blue ***/
								ShowImage (imggblue[iImgDir], iImgX, iImgY,
									"imggblue[]", ascreen, iScale, 1); break;
							case 7: /*** lgreen ***/
								ShowImage (imgglgreen[iImgDir], iImgX, iImgY,
									"imgglgreen[]", ascreen, iScale, 1); break;
							default: /*** unknown ***/
								ShowImage (imggunknown[iImgDir], iImgX, iImgY,
									"imggunknown[]", ascreen, iScale, 1);
								printf ("[ WARN ] Strange guard color: %i!\n",
									sGuardColors[iCurRoom - 1]);
								break;
						}
						if (iSelected == iLoc)
						{
							ShowImage (imggsel[iImgDir], iImgX, iImgY,
								"imggsel[]", ascreen, iScale, 1);
						}
						break;
					case 1: /*** fat ***/
						ShowImage (imgfat[iImgDir], iImgX, iImgY,
							"imgfat[]", ascreen, iScale, 1);
						if (iSelected == iLoc)
						{
							ShowImage (imgfatsel[iImgDir], iImgX, iImgY,
							"imgfatsel[]", ascreen, iScale, 1);
						}
						break;
					case 2: /*** skeleton ***/
						ShowImage (imgskel[iImgDir], iImgX, iImgY,
							"imgskel[]", ascreen, iScale, 1);
						if (iSelected == iLoc)
						{
							ShowImage (imgskelsel[iImgDir], iImgX, iImgY,
							"imgskelsel[]", ascreen, iScale, 1);
						}
						break;
					case 3: /*** Jaffar ***/
						ShowImage (imgjaffar[iImgDir], iImgX, iImgY,
							"imgjaffar[]", ascreen, iScale, 1);
						if (iSelected == iLoc)
						{
							ShowImage (imgjaffarsel[iImgDir], iImgX, iImgY,
							"imgjaffarsel[]", ascreen, iScale, 1);
						}
						break;
					case 4: /*** shadow ***/
						ShowImage (imgshadow[iImgDir], iImgX, iImgY,
							"imgshadow[]", ascreen, iScale, 1);
						if (iSelected == iLoc)
						{
							ShowImage (imgshadowsel[iImgDir], iImgX, iImgY,
							"imgshadowsel[]", ascreen, iScale, 1);
						}
						break;
				}

				switch (sGuardSkills[iCurRoom - 1])
				{
					case 10:
						snprintf (arText[0], MAX_TEXT, "%s", "LVL:a"); break;
					case 11:
						snprintf (arText[0], MAX_TEXT, "%s", "LVL:b"); break;
					default:
						snprintf (arText[0], MAX_TEXT, "LVL:%i",
							sGuardSkills[iCurRoom - 1]); break;
				}
				DisplayText (iXY[0] + 7, iXY[1] + 151, 11, arText, 1,
					font11, color_wh, color_bl, 0);
			}
		}

		/*** above this room ***/
		for (iLoopTile = 1; iLoopTile <= 10; iLoopTile++)
		{
			if (iRoomConnections[iCurRoom][3] != 0)
			{
				ShowTile (iThingA[iRoomConnections[iCurRoom][3]][20 + iLoopTile],
					iModifierA[iRoomConnections[iCurRoom][3]][20 + iLoopTile],
					-11 + iLoopTile);
			} else {
				ShowTile (0x01, 0x00, -11 + iLoopTile);
			}
		}
	}
	if (iScreenS == 2) /*** R ***/
	{
		if (iBrokenRoomLinks == 0)
		{
			InitRooms();
			/*** room links ***/
			ShowImage (imgrl, 25, 50, "imgrl", ascreen, iScale, 1);
			/*** Map window ***/
			if (iMapOpen == 0)
			{
				if (iDownAt == 12)
				{
					ShowImage (imgmapon_1, 596, 65, "imgmapon_1",
						ascreen, iScale, 1);
				} else {
					ShowImage (imgmapon_0, 596, 65, "imgmapon_0",
						ascreen, iScale, 1);
				}
			} else {
				ShowImage (imgmapoff, 596, 65, "imgmapoff", ascreen, iScale, 1);
			}
			/*** rooms broken on ***/
			if (iDownAt == 11)
			{ /*** down ***/
				ShowImage (imgbroomson_1, 626, 65, "imgbroomson_1", ascreen, iScale, 1);
			} else { /*** up ***/
				ShowImage (imgbroomson_0, 626, 65, "imgbroomson_0", ascreen, iScale, 1);
			}
			WhereToStart();
			for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
			{
				iDone[iLoopRoom] = 0;
			}
			ShowRooms ((int)luKidRoom, iStartRoomsX, iStartRoomsY, 1);
			iUnusedRooms = 0;
			for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
			{
				if (iDone[iLoopRoom] != 1)
				{
					iUnusedRooms++;
					ShowRooms (iLoopRoom, 25, iUnusedRooms, 0);
				}
			}
			if (iMovingRoom != 0)
			{
				iXShow = (iXPos + 10) / iScale;
				iYShow = (iYPos + 10) / iScale;
				ShowImage (imgroom[iMovingRoom], iXShow, iYShow, "imgroom[]",
					ascreen, iScale, 1);
				if (iCurRoom == iMovingRoom) /*** green stripes ***/
					{ ShowImage (imgsrc, iXShow, iYShow, "imgsrc", ascreen, iScale, 1); }
				if ((int)luKidRoom == iMovingRoom) /*** blue border ***/
					{ ShowImage (imgsrs, iXShow, iYShow, "imgsrs", ascreen, iScale, 1); }
				ShowImage (imgsrm, iXShow, iYShow, "imgsrm", ascreen, iScale, 1);
				ShowRooms (-1, iMovingNewX, iMovingNewY, 0);
			}
		} else {
			/*** broken room links ***/
			ShowImage (imgbrl, 25, 50, "imgbrl", ascreen, iScale, 1);
			for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
			{
				switch (iLoopRoom)
				{
					case 1: iXShow = (63 * 0); iYShow = (63 * 0); break;
					case 2: iXShow = (63 * 1); iYShow = (63 * 0); break;
					case 3: iXShow = (63 * 2); iYShow = (63 * 0); break;
					case 4: iXShow = (63 * 3); iYShow = (63 * 0); break;
					case 5: iXShow = (63 * 0); iYShow = (63 * 1); break;
					case 6: iXShow = (63 * 1); iYShow = (63 * 1); break;
					case 7: iXShow = (63 * 2); iYShow = (63 * 1); break;
					case 8: iXShow = (63 * 3); iYShow = (63 * 1); break;
					case 9: iXShow = (63 * 0); iYShow = (63 * 2); break;
					case 10: iXShow = (63 * 1); iYShow = (63 * 2); break;
					case 11: iXShow = (63 * 2); iYShow = (63 * 2); break;
					case 12: iXShow = (63 * 3); iYShow = (63 * 2); break;
					case 13: iXShow = (63 * 0); iYShow = (63 * 3); break;
					case 14: iXShow = (63 * 1); iYShow = (63 * 3); break;
					case 15: iXShow = (63 * 2); iYShow = (63 * 3); break;
					case 16: iXShow = (63 * 3); iYShow = (63 * 3); break;
					case 17: iXShow = (63 * 0); iYShow = (63 * 4); break;
					case 18: iXShow = (63 * 1); iYShow = (63 * 4); break;
					case 19: iXShow = (63 * 2); iYShow = (63 * 4); break;
					case 20: iXShow = (63 * 3); iYShow = (63 * 4); break;
					case 21: iXShow = (63 * 0); iYShow = (63 * 5); break;
					case 22: iXShow = (63 * 1); iYShow = (63 * 5); break;
					case 23: iXShow = (63 * 2); iYShow = (63 * 5); break;
					case 24: iXShow = (63 * 3); iYShow = (63 * 5); break;
				}
				if (iCurRoom == iLoopRoom)
				{ /*** green stripes ***/
					ShowImage (imgsrc, iXShow + broken_room_x,
						iYShow + broken_room_y, "imgsrc", ascreen, iScale, 1);
				}
				if ((int)luKidRoom == iLoopRoom)
				{ /*** blue border ***/
					ShowImage (imgsrs, iXShow + broken_room_x,
						iYShow + broken_room_y, "imgsrs", ascreen, iScale, 1);
				}
				for (iLoopSide = 1; iLoopSide <= 4; iLoopSide++)
				{
					if (iRoomConnections[iLoopRoom][iLoopSide] != 0)
					{
						snprintf (sShowRoom, MAX_TEXT, "imgroom[%i]",
							iRoomConnections[iLoopRoom][iLoopSide]);
						ShowImage (imgroom[iRoomConnections[iLoopRoom][iLoopSide]],
							iXShow + broken_side_x[iLoopSide],
							iYShow + broken_side_y[iLoopSide],
							sShowRoom, ascreen, iScale, 1);

						if (iRoomConnectionsBroken[iLoopRoom][iLoopSide] == 1)
						{ /*** blue square ***/
							ShowImage (imgsrb,
								iXShow + broken_side_x[iLoopSide],
								iYShow + broken_side_y[iLoopSide],
								"imgsrb", ascreen, iScale, 1);
						}
					}

					if ((iChangingBrokenRoom == iLoopRoom) &&
						(iChangingBrokenSide == iLoopSide))
					{ /*** red stripes ***/
						ShowImage (imgsrm,
							iXShow + broken_side_x[iLoopSide],
							iYShow + broken_side_y[iLoopSide],
							"imgsrm", ascreen, iScale, 1);
					}
				}
			}
		}
	}
	if (iScreenS == 3) /*** E ***/
	{
		iEventUnused = 0;

		/*** events screen ***/
		ShowImage (imgevents, 25, 50, "imgevents", ascreen, iScale, 1);

		/*** current event number ***/
		CenterNumber (ascreen, iChangeEvent + 1, 368, 62, color_wh, 0);

		/*** sel event, room ***/
		iERoom = EventInfo (iChangeEvent, 1);
		if ((iERoom >= 1) && (iERoom <= ROOMS))
		{
			iShowX = 279 + (15 * iERoom);
			iShowY = 117;
			ShowImage (imgsele, iShowX, iShowY, "imgsele", ascreen, iScale, 1);
		} else { iEventUnused = 1; }

		/*** sel event, tile ***/
		iETile = EventInfo (iChangeEvent, 2);
		if ((iETile >= 1) && (iETile <= TILES))
		{
			iShowX = -10; iShowY = -10; /*** To prevent warnings. ***/
			if ((iETile >= 1) && (iETile <= 10))
			{
				iShowX = 444 + (15 * iETile);
				iShowY = 157;
			}
			if ((iETile >= 11) && (iETile <= 20))
			{
				iShowX = 444 + (15 * (iETile - 10));
				iShowY = 172;
			}
			if ((iETile >= 21) && (iETile <= 30))
			{
				iShowX = 444 + (15 * (iETile - 20));
				iShowY = 187;
			}
			ShowImage (imgsele, iShowX, iShowY, "imgsele", ascreen, iScale, 1);
		} else { iEventUnused = 1; }

		/*** sel event, next ***/
		iENext = EventInfo (iChangeEvent, 3);
		if ((iENext == 0) || (iENext == 1))
		{
			switch (iENext)
			{
				case 0: iShowX = 579; break; /*** n ***/
				case 1: iShowX = 594; break; /*** y ***/
			}
			ShowImage (imgsele, iShowX, 227, "imgsele", ascreen, iScale, 1);
		}

		/*** Show target. ***/
		if (iEventUnused == 0)
		{
			ShowTile (iThingA[iERoom][iETile], iModifierA[iERoom][iETile], 104);
		} else {
			ShowImage (imgeventu, 64, 130, "imgeventu", ascreen, iScale, 1);
		}

		if (iMouse == 1)
			{ ShowImage (imgmouse, 608, 71, "imgmouse", ascreen, iScale, 1); }
	}

	if (iRoomConnections[iCurRoom][1] != 0)
	{ /*** left ***/
		if (iDownAt == 1)
		{ /*** down ***/
			ShowImage (imgleft_1, 0, 50, "imgleft_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgleft_0, 0, 50, "imgleft_0", ascreen, iScale, 1);
		}
	} else { /*** no left ***/
		ShowImage (imglrno, 0, 50, "imglrno", ascreen, iScale, 1);
	}
	if (iRoomConnections[iCurRoom][2] != 0)
	{ /*** right ***/
		if (iDownAt == 2)
		{ /*** down ***/
			ShowImage (imgright_1, 665, 50, "imgright_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgright_0, 665, 50, "imgright_0", ascreen, iScale, 1);
		}
	} else { /*** no right ***/
		ShowImage (imglrno, 665, 50, "imglrno", ascreen, iScale, 1);
	}
	if (iRoomConnections[iCurRoom][3] != 0)
	{ /*** up ***/
		if (iDownAt == 3)
		{ /*** down ***/
			ShowImage (imgup_1, 25, 25, "imgup_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgup_0, 25, 25, "imgup_0", ascreen, iScale, 1);
		}
	} else { /*** no up ***/
		if (iScreen != 1)
		{ /*** without info ***/
			ShowImage (imgudno, 25, 25, "imgudno", ascreen, iScale, 1);
		} else { /*** with info ***/
			ShowImage (imgudnonfo, 25, 25, "imgudnonfo", ascreen, iScale, 1);
		}
	}
	if (iRoomConnections[iCurRoom][4] != 0)
	{ /*** down ***/
		if (iDownAt == 4)
		{ /*** down ***/
			ShowImage (imgdown_1, 25, 434, "imgdown_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgdown_0, 25, 434, "imgdown_0", ascreen, iScale, 1);
		}
	} else { /*** no down ***/
		ShowImage (imgudno, 25, 434, "imgudno", ascreen, iScale, 1);
	}

	/*** room links ***/
	if (iScreen != 2)
	{
		if (iBrokenRoomLinks == 1)
		{
			/*** broken room links ***/
			if (iDownAt == 11)
			{ /*** down ***/
				ShowImage (imgbroomson_1, 0, 25, "imgbroomson_1", ascreen, iScale, 1);
			} else { /*** up ***/
				ShowImage (imgbroomson_0, 0, 25, "imgbroomson_0", ascreen, iScale, 1);
			}
		} else {
			/*** regular room links ***/
			if (iDownAt == 5)
			{ /*** down ***/
				ShowImage (imgroomson_1, 0, 25, "imgroomson_1", ascreen, iScale, 1);
			} else { /*** up ***/
				ShowImage (imgroomson_0, 0, 25, "imgroomson_0", ascreen, iScale, 1);
			}
		}
	} else {
		if (iBrokenRoomLinks == 1)
		{
			/*** broken room links ***/
			ShowImage (imgbroomsoff, 0, 25, "imgbroomsoff", ascreen, iScale, 1);
		} else {
			/*** regular room links ***/
			ShowImage (imgroomsoff, 0, 25, "imgroomsoff", ascreen, iScale, 1);
		}
	}

	/*** events ***/
	if (iScreen != 3)
	{
		if (iDownAt == 6)
		{ /*** down ***/
			ShowImage (imgeventson_1, 665, 25, "imgeventson_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgeventson_0, 665, 25, "imgeventson_0", ascreen, iScale, 1);
		}
	} else { /*** off ***/
		ShowImage (imgeventsoff, 665, 25, "imgeventsoff", ascreen, iScale, 1);
	}

	/*** save ***/
	if (iChanged != 0)
	{ /*** on ***/
		if (iDownAt == 7)
		{ /*** down ***/
			ShowImage (imgsaveon_1, 0, 434, "imgsaveon_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgsaveon_0, 0, 434, "imgsaveon_0", ascreen, iScale, 1);
		}
	} else { /*** off ***/
		ShowImage (imgsaveoff, 0, 434, "imgsaveoff", ascreen, iScale, 1);
	}

	/*** quit ***/
	if (iDownAt == 8)
	{ /*** down ***/
		ShowImage (imgquit_1, 665, 434, "imgquit_1", ascreen, iScale, 1);
	} else { /*** up ***/
		ShowImage (imgquit_0, 665, 434, "imgquit_0", ascreen, iScale, 1);
	}

	/*** prev ***/
	if (iCurLevel != 1)
	{ /*** on ***/
		if (iDownAt == 9)
		{ /*** down ***/
			ShowImage (imgprevon_1, 0, 0, "imgprevon_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgprevon_0, 0, 0, "imgprevon_0", ascreen, iScale, 1);
		}
	} else { /*** off ***/
		ShowImage (imgprevoff, 0, 0, "imgprevoff", ascreen, iScale, 1);
	}

	/*** next ***/
	if (iCurLevel != 0)
	{ /*** on ***/
		if (iDownAt == 10)
		{ /*** down ***/
			ShowImage (imgnexton_1, 665, 0, "imgnexton_1", ascreen, iScale, 1);
		} else { /*** up ***/
			ShowImage (imgnexton_0, 665, 0, "imgnexton_0", ascreen, iScale, 1);
		}
	} else { /*** off ***/
		ShowImage (imgnextoff, 665, 0, "imgnextoff", ascreen, iScale, 1);
	}

	/*** level bar ***/
	ShowImage (imgbar, 25, 0, "imgbar", ascreen, iScale, 1);
	switch (iCurLevel)
	{
		case 0: snprintf (sLevel, MAX_TEXT, "%s", "level 0 (demo)"); break;
		case 1: snprintf (sLevel, MAX_TEXT, "%s", "level 1 (prison)"); break;
		case 2: snprintf (sLevel, MAX_TEXT, "%s", "level 2 (guards)"); break;
		case 3: snprintf (sLevel, MAX_TEXT, "%s", "level 3 (skeleton)"); break;
		case 4: snprintf (sLevel, MAX_TEXT, "%s", "level 4 (mirror)"); break;
		case 5: snprintf (sLevel, MAX_TEXT, "%s", "level 5 (thief)"); break;
		case 6: snprintf (sLevel, MAX_TEXT, "%s", "level 6 (plunge)"); break;
		case 7: snprintf (sLevel, MAX_TEXT, "%s", "level 7 (weightless)"); break;
		case 8: snprintf (sLevel, MAX_TEXT, "%s", "level 8 (mouse)"); break;
		case 9: snprintf (sLevel, MAX_TEXT, "%s", "level 9 (twisty)"); break;
		case 10: snprintf (sLevel, MAX_TEXT, "%s", "level 10 (quad)"); break;
		case 11: snprintf (sLevel, MAX_TEXT, "%s", "level 11 (fragile)"); break;
		case 12: snprintf (sLevel, MAX_TEXT, "%s", "level 12 (12a; tower)"); break;
		case 13: snprintf (sLevel, MAX_TEXT, "%s",
			"level 13 (12b; jaffar)"); break;
		case 14: snprintf (sLevel, MAX_TEXT, "%s", "level 14 (rescue)"); break;
		case 15: snprintf (sLevel, MAX_TEXT, "%s", "level 15 (potions)"); break;
	}
	switch (iScreen)
	{
		case 1:
			snprintf (arText[0], MAX_TEXT, "%s, room %i", sLevel, iCurRoom);
			ShowImage (extras[iExtras], 608, 3, "extras[iExtras]",
				ascreen, iScale, 1);
			break;
		case 2:
			snprintf (arText[0], MAX_TEXT, "%s, room links", sLevel); break;
		case 3:
			snprintf (arText[0], MAX_TEXT, "%s, events", sLevel); break;
	}
	DisplayText (31, 5, 15, arText, 1,
		font15, color_wh, color_bl, 0);
	if (iPlaytest == 1)
		{ ShowImage (imgplaytest, 25, 50, "imgplaytest", ascreen, iScale, 1); }

	ShowMap();

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void InitPopUp (void)
/*****************************************************************************/
{
	SDL_Event event;
	int iPopUp;

	iPopUp = 1;

	PlaySound ("wav/popup.wav");
	ShowPopUp();
	SDL_RaiseWindow (window);

	while (iPopUp == 1)
	{
		while (SDL_PollEvent (&event))
		{
			if (MapEvents (event) == 0)
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							iPopUp = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_o:
							iPopUp = 0; break;
					}
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (439, 319, 439 + 85, 319 + 32) == 1) /*** OK ***/
						{
							if (iOKOn != 1) { iOKOn = 1; }
							ShowPopUp();
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					iOKOn = 0;
					if (event.button.button == 1)
					{
						if (InArea (439, 319, 439 + 85, 319 + 32) == 1) /*** OK ***/
						{
							iPopUp = 0;
						}
					}
					ShowPopUp(); break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED:
							ShowScreen (iScreen); ShowPopUp(); break;
						case SDL_WINDOWEVENT_CLOSE:
							Quit(); break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							iActiveWindowID = iWindowID; break;
					}
					break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	ShowScreen (iScreen);
}
/*****************************************************************************/
void ShowPopUp (void)
/*****************************************************************************/
{
	char arText[9 + 2][MAX_TEXT + 2];

	/*** faded background ***/
	ShowImage (imgfaded, 0, 0, "imgfaded", ascreen, iScale, 1);
	/*** popup ***/
	ShowImage (imgpopup, 84, 9, "imgpopup", ascreen, iScale, 1);
	switch (iOKOn)
	{
		case 0:
			/*** OK off ***/
			ShowImage (imgok[1], 439, 319, "imgok[1]", ascreen, iScale, 1); break;
		case 1:
			/*** OK on ***/
			ShowImage (imgok[2], 439, 319, "imgok[2]", ascreen, iScale, 1); break;
	}

	snprintf (arText[0], MAX_TEXT, "%s %s", EDITOR_NAME, EDITOR_VERSION);
	snprintf (arText[1], MAX_TEXT, "%s", COPYRIGHT);
	snprintf (arText[2], MAX_TEXT, "%s", "");
	if (iController != 1)
	{
		snprintf (arText[3], MAX_TEXT, "%s", "single tile (change or select)");
		snprintf (arText[4], MAX_TEXT, "%s", "entire room (clear or fill)");
		snprintf (arText[5], MAX_TEXT, "%s", "entire level (randomize or fill)");
	} else {
		snprintf (arText[3], MAX_TEXT, "%s", "The detected game controller:");
		snprintf (arText[4], MAX_TEXT, "%s", sControllerName);
		snprintf (arText[5], MAX_TEXT, "%s", "Have fun using it!");
	}
	snprintf (arText[6], MAX_TEXT, "%s", "");
	snprintf (arText[7], MAX_TEXT, "%s", "You may use one guard per room.");
	snprintf (arText[8], MAX_TEXT, "%s", "The tile behavior may differ per"
		" level.");

	DisplayText (179, 120, 15, arText, 9,
		font15, color_wh, color_bl, 0);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
int MapEvents (SDL_Event event)
/*****************************************************************************/
{
	int iIsMapEvent;

	iIsMapEvent = 0;

	switch (event.type)
	{
		case SDL_CONTROLLERBUTTONDOWN:
			/*** This event has no windowID. ***/
			if (iActiveWindowID == iWindowMapID)
			{
				MapControllerDown (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_CONTROLLERBUTTONUP:
			/*** This event has no windowID. ***/
			if (iActiveWindowID == iWindowMapID)
			{
				MapControllerUp (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			/*** This event has no windowID. ***/
			if (iActiveWindowID == iWindowMapID)
			{
				MapControllerMotion (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_KEYDOWN:
			if (event.key.windowID == iWindowMapID)
			{
				MapKeyDown (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_MOUSEMOTION:
			if (event.motion.windowID == iWindowMapID)
			{
				MapMouseMotion (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.windowID == iWindowMapID)
			{
				MapButtonDown (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.windowID == iWindowMapID)
			{
				MapButtonUp (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_MOUSEWHEEL:
			if (event.wheel.windowID == iWindowMapID)
			{
				MapMouseWheel (event);
				iIsMapEvent = 1;
			}
			break;
		case SDL_WINDOWEVENT:
			if (event.window.windowID == iWindowMapID)
			{
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_EXPOSED:
						ShowMap(); break;
					case SDL_WINDOWEVENT_CLOSE:
						MapHide(); break;
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						iActiveWindowID = iWindowMapID; break;
				}
				iIsMapEvent = 1;
			}
			break;
		case SDL_QUIT:
			/*** This event has no windowID, I think. ***/
			if (iActiveWindowID == iWindowMapID)
			{
				Quit();
				iIsMapEvent = 1;
			}
			break;
	}

	return (iIsMapEvent);
}
/*****************************************************************************/
void Quit (void)
/*****************************************************************************/
{
	if (iChanged != 0) { InitPopUpSave(); }
	if (iModified == 1) { ModifyBack(); }
	TTF_CloseFont (font11);
	TTF_CloseFont (font15);
	TTF_CloseFont (font20);
	TTF_Quit();
	SDL_Quit();
	exit (EXIT_NORMAL);
}
/*****************************************************************************/
void PlaySound (char *sFile)
/*****************************************************************************/
{
	int iIndex;
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT cvt;

	if (iNoAudio == 1) { return; }
	for (iIndex = 0; iIndex < NUM_SOUNDS; iIndex++)
	{
		if (sounds[iIndex].dpos == sounds[iIndex].dlen)
		{
			break;
		}
	}
	if (iIndex == NUM_SOUNDS) { return; }

	if (SDL_LoadWAV (sFile, &wave, &data, &dlen) == NULL)
	{
		printf ("[FAILED] Could not load %s: %s!\n", sFile, SDL_GetError());
		exit (EXIT_ERROR);
	}
	SDL_BuildAudioCVT (&cvt, wave.format, wave.channels, wave.freq, AUDIO_S16, 2,
		44100);
	/*** The "+ 1" is a workaround for SDL bug #2274. ***/
	cvt.buf = (Uint8 *)malloc (dlen * (cvt.len_mult + 1));
	memcpy (cvt.buf, data, dlen);
	cvt.len = dlen;
	SDL_ConvertAudio (&cvt);
	SDL_FreeWAV (data);

	if (sounds[iIndex].data)
	{
		free (sounds[iIndex].data);
	}
	SDL_LockAudio();
	sounds[iIndex].data = cvt.buf;
	sounds[iIndex].dlen = cvt.len_cvt;
	sounds[iIndex].dpos = 0;
	SDL_UnlockAudio();
}
/*****************************************************************************/
int InArea (int iUpperLeftX, int iUpperLeftY,
	int iLowerRightX, int iLowerRightY)
/*****************************************************************************/
{
	if ((iUpperLeftX * iScale <= iXPos) &&
		(iLowerRightX * iScale >= iXPos) &&
		(iUpperLeftY * iScale <= iYPos) &&
		(iLowerRightY * iScale >= iYPos))
	{
		return (1);
	} else {
		return (0);
	}
}
/*****************************************************************************/
int InAreaMap (int iUpperLeftX, int iUpperLeftY,
	int iLowerRightX, int iLowerRightY)
/*****************************************************************************/
{
	if ((iUpperLeftX <= iXPosMap) &&
		(iLowerRightX >= iXPosMap) &&
		(iUpperLeftY <= iYPosMap) &&
		(iLowerRightY >= iYPosMap))
	{
		return (1);
	} else {
		return (0);
	}
}
/*****************************************************************************/
void InitPopUpSave (void)
/*****************************************************************************/
{
	int iPopUp;
	SDL_Event event;

	iPopUp = 1;

	PlaySound ("wav/popup_yn.wav");
	ShowPopUpSave();
	SDL_RaiseWindow (window);

	while (iPopUp == 1)
	{
		while (SDL_PollEvent (&event))
		{
			if (MapEvents (event) == 0)
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							SaveLevel (iCurLevel); iPopUp = 0; break;
						case SDL_CONTROLLER_BUTTON_B:
							iPopUp = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						case SDLK_n:
							iPopUp = 0; break;
						case SDLK_y:
							SaveLevel (iCurLevel); iPopUp = 0; break;
					}
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (166, 319, 166 + 85, 319 + 32) == 1) { iNoOn = 1; }
						if (InArea (439, 319, 439 + 85, 319 + 32) == 1) { iYesOn = 1; }
					}
					ShowPopUpSave();
					break;
				case SDL_MOUSEBUTTONUP:
					iNoOn = 0;
					iYesOn = 0;
					if (event.button.button == 1)
					{
						if (InArea (166, 319, 166 + 85, 319 + 32) == 1)
							{ iPopUp = 0; }
						if (InArea (439, 319, 439 + 85, 319 + 32) == 1)
							{ SaveLevel (iCurLevel); iPopUp = 0; }
					}
					ShowPopUpSave();
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED:
							ShowScreen (iScreen); ShowPopUpSave(); break;
						case SDL_WINDOWEVENT_CLOSE:
							Quit(); break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							iActiveWindowID = iWindowID; break;
					}
					break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	ShowScreen (iScreen);
}
/*****************************************************************************/
void ShowPopUpSave (void)
/*****************************************************************************/
{
	char arText[2 + 2][MAX_TEXT + 2];

	/*** faded background ***/
	ShowImage (imgfaded, 0, 0, "imgfaded", ascreen, iScale, 1);
	/*** popup_yn ***/
	ShowImage (imgpopup_yn, 149, 91, "imgpopup_yn", ascreen, iScale, 1);

	switch (iNoOn)
	{
		case 0: /*** off ***/
			ShowImage (imgno[1], 166, 319, "imgno[1]", ascreen, iScale, 1);
			break;
		case 1: /*** on ***/
			ShowImage (imgno[2], 166, 319, "imgno[2]", ascreen, iScale, 1);
			break;
	}
	switch (iYesOn)
	{
		case 0: /*** off ***/
			ShowImage (imgyes[1], 439, 319, "imgyes[1]", ascreen, iScale, 1);
			break;
		case 1: /*** on ***/
			ShowImage (imgyes[2], 439, 319, "imgyes[2]", ascreen, iScale, 1);
			break;
	}

	if (iChanged == 1)
	{
		snprintf (arText[0], MAX_TEXT, "%s", "You made an unsaved change.");
		snprintf (arText[1], MAX_TEXT, "%s", "Do you want to save it?");
	} else {
		snprintf (arText[0], MAX_TEXT, "There are unsaved changes.");
		snprintf (arText[1], MAX_TEXT, "%s", "Do you wish to save these?");
	}
	DisplayText (179, 120, 15, arText, 2, font15, color_wh, color_bl, 0);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void DisplayText (int iStartX, int iStartY, int iFontSize,
	char arText[9 + 2][MAX_TEXT + 2], int iLines, TTF_Font *font,
	SDL_Color back, SDL_Color fore, int iOnMap)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iLoopLine;

	for (iLoopLine = 0; iLoopLine < iLines; iLoopLine++)
	{
		if (strcmp (arText[iLoopLine], "") != 0)
		{
			message = TTF_RenderText_Shaded (font,
				arText[iLoopLine], fore, back);
			if (iOnMap == 0)
			{
				messaget = SDL_CreateTextureFromSurface (ascreen, message);
			} else {
				messaget = SDL_CreateTextureFromSurface (mscreen, message);
			}
			if ((strcmp (arText[iLoopLine], "single tile (change or select)") == 0)
				|| (strcmp (arText[iLoopLine], "entire room (clear or fill)") == 0) ||
				(strcmp (arText[iLoopLine], "entire level (randomize or fill)") == 0))
			{
				offset.x = iStartX + 20;
			} else {
				offset.x = iStartX;
			}
			offset.y = iStartY + (iLoopLine * (iFontSize + 4));
			if (iOnMap == 0)
			{
				offset.w = message->w; offset.h = message->h;
				CustomRenderCopy (messaget, "message", NULL, ascreen, &offset);
			} else {
				offset.w = message->w / iScale; offset.h = message->h / iScale;
				CustomRenderCopy (messaget, "map message", NULL, mscreen, &offset);
			}
			SDL_DestroyTexture (messaget); SDL_FreeSurface (message);
		}
	}
}
/*****************************************************************************/
void CustomRenderCopy (SDL_Texture* src, char *sImageInfo, SDL_Rect* srcrect,
	SDL_Renderer* dst, SDL_Rect *dstrect)
/*****************************************************************************/
{
	SDL_Rect stuff;

	if (strcmp (sImageInfo, "map message") == 0)
	{
		stuff.x = dstrect->x;
		stuff.y = dstrect->y;
	} else {
		stuff.x = dstrect->x * iScale;
		stuff.y = dstrect->y * iScale;
	}
	if (srcrect != NULL) /*** image ***/
	{
		stuff.w = dstrect->w * iScale;
		stuff.h = dstrect->h * iScale;
	} else { /*** font ***/
		stuff.w = dstrect->w;
		stuff.h = dstrect->h;
	}
	if (SDL_RenderCopy (dst, src, srcrect, &stuff) != 0)
	{
		printf ("[ WARN ] SDL_RenderCopy (%s): %s!\n",
			sImageInfo, SDL_GetError());
	}
}
/*****************************************************************************/
void GetForegroundAsName (char *sBinaryFore, char *sName)
/*****************************************************************************/
{
	char sPart[6 + 2];

	strncpy (sPart, sBinaryFore + 3, 5);
	sPart[5] = '\0';
	if (strcmp (sPart, "00000") == 0) { snprintf (sName, 9, "%s", "Empty  "); }
	if (strcmp (sPart, "00001") == 0) { snprintf (sName, 9, "%s", "Floor  "); }
	if (strcmp (sPart, "00010") == 0) { snprintf (sName, 9, "%s", "Spikes "); }
	if (strcmp (sPart, "00011") == 0) { snprintf (sName, 9, "%s", "Pillar "); }
	if (strcmp (sPart, "00100") == 0) { snprintf (sName, 9, "%s", "Gate   "); }
	if (strcmp (sPart, "00101") == 0) { snprintf (sName, 9, "%s", "StckBtn"); }
	if (strcmp (sPart, "00110") == 0) { snprintf (sName, 9, "%s", "DropBtn"); }
	if (strcmp (sPart, "00111") == 0) { snprintf (sName, 9, "%s", "Tpstry "); }
	if (strcmp (sPart, "01000") == 0) { snprintf (sName, 9, "%s", "BBgpilr"); }
	if (strcmp (sPart, "01001") == 0) { snprintf (sName, 9, "%s", "TBgpilr"); }
	if (strcmp (sPart, "01010") == 0) { snprintf (sName, 9, "%s", "Potion "); }
	if (strcmp (sPart, "01011") == 0)
	{
		if (sBinaryFore[2] == '1')
			{ snprintf (sName, 9, "%s", "LoosStk"); }
				else { snprintf (sName, 9, "%s", "LoosBrd"); }
	}
	if (strcmp (sPart, "01100") == 0) { snprintf (sName, 9, "%s", "TpstryT"); }
	if (strcmp (sPart, "01101") == 0) { snprintf (sName, 9, "%s", "Mirror "); }
	if (strcmp (sPart, "01110") == 0) { snprintf (sName, 9, "%s", "Debris "); }
	if (strcmp (sPart, "01111") == 0) { snprintf (sName, 9, "%s", "RaisBtn"); }
	if (strcmp (sPart, "10000") == 0) { snprintf (sName, 9, "%s", "ExitLft"); }
	if (strcmp (sPart, "10001") == 0) { snprintf (sName, 9, "%s", "ExitRgt"); }
	if (strcmp (sPart, "10010") == 0) { snprintf (sName, 9, "%s", "Chomper"); }
	if (strcmp (sPart, "10011") == 0) { snprintf (sName, 9, "%s", "Torch  "); }
	if (strcmp (sPart, "10100") == 0) { snprintf (sName, 9, "%s", "Wall   "); }
	if (strcmp (sPart, "10101") == 0) { snprintf (sName, 9, "%s", "Skeletn"); }
	if (strcmp (sPart, "10110") == 0) { snprintf (sName, 9, "%s", "Sword  "); }
	if (strcmp (sPart, "10111") == 0) { snprintf (sName, 9, "%s", "BcnyLft"); }
	if (strcmp (sPart, "11000") == 0) { snprintf (sName, 9, "%s", "BcnyRgt"); }
	if (strcmp (sPart, "11001") == 0) { snprintf (sName, 9, "%s", "LcePilr"); }
	if (strcmp (sPart, "11010") == 0) { snprintf (sName, 9, "%s", "LceSprt"); }
	if (strcmp (sPart, "11011") == 0) { snprintf (sName, 9, "%s", "SmalLce"); }
	if (strcmp (sPart, "11100") == 0) { snprintf (sName, 9, "%s", "LceLft "); }
	if (strcmp (sPart, "11101") == 0) { snprintf (sName, 9, "%s", "LceRgt "); }
	if (strcmp (sPart, "11110") == 0) { snprintf (sName, 9, "%s", "TorchDb"); }
	if (strcmp (sPart, "11111") == 0) { snprintf (sName, 9, "%s", "Null   "); }
}
/*****************************************************************************/
void ShowTile (int iThing, int iModifier, int iLocation)
/*****************************************************************************/
{
	int iThingShow, iModifierShow;
	char sShowTile[MAX_TEXT + 2];
	int iXY[2 + 2];
	int iShown;
	int iInfoC;
	char arText[1 + 2][MAX_TEXT + 2];

	iThingShow = iThing;
	if ((iThingShow > 32) && (iThingShow != 0x2B)) { iThingShow-=32; }
	iModifierShow = iModifier;
	if ((iThingShow == 0x06) || (iThingShow == 0x0F)) { iModifierShow = 0x00; }

	LocationToXY (iLocation, iXY);

	iInfoC = 0;
	if (cCurType == 'd')
	{
		iShown = 0;
		if (iNoAnim == 0)
		{
			if ((iThingShow == 0x13) && (iModifierShow == 0x00))
			{
				ShowImage (spriteflamed1, iXY[0], iXY[1],
					"spriteflamed1", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x1E) && (iModifierShow == 0x00))
			{
				ShowImage (spriteflamed2, iXY[0], iXY[1],
					"spriteflamed2", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x12) && (iModifierShow == 0x00))
			{
				ShowImage (spritechomperd1, iXY[0], iXY[1],
					"spritechomperd1", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x12) && (iModifierShow == 0x80))
			{
				ShowImage (spritechomperd2, iXY[0], iXY[1],
					"spritechomperd2", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x16) && (iModifierShow == 0x00))
			{
				ShowImage (spriteswordd, iXY[0], iXY[1],
					"spriteswordd", ascreen, iScale, 1); iShown = 1;
			}
		}
		if (iShown == 0)
		{
			if (imgdungeon[iThingShow][iModifierShow] != NULL)
			{
				snprintf (sShowTile, MAX_TEXT, "imgdungeon[%i][%i]",
					iThingShow, iModifierShow);
				ShowImage (imgdungeon[iThingShow][iModifierShow], iXY[0], iXY[1],
					sShowTile, ascreen, iScale, 1);
			} else {
				ShowImage (imgunk[1], iXY[0], iXY[1], "imgunk[1]",
					ascreen, iScale, 1);
				iInfoC = 1;
			}
		}
	} else {
		iShown = 0;
		if (iNoAnim == 0)
		{
			if ((iThingShow == 0x13) && (iModifierShow == 0x00))
			{
				ShowImage (spriteflamep1, iXY[0], iXY[1],
					"spriteflamep1", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x1E) && (iModifierShow == 0x00))
			{
				ShowImage (spriteflamep2, iXY[0], iXY[1],
					"spriteflamep2", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x12) && (iModifierShow == 0x00))
			{
				ShowImage (spritechomperp1, iXY[0], iXY[1],
					"spritechomperp1", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x12) && (iModifierShow == 0x80))
			{
				ShowImage (spritechomperp2, iXY[0], iXY[1],
					"spritechomperp2", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x16) && (iModifierShow == 0x00))
			{
				ShowImage (spriteswordp, iXY[0], iXY[1],
					"spriteswordp", ascreen, iScale, 1); iShown = 1;
			}
		}
		if (iShown == 0)
		{
			if (imgpalace[iThingShow][iModifierShow] != NULL)
			{
				snprintf (sShowTile, MAX_TEXT, "imgpalace[%i][%i]",
					iThingShow, iModifierShow);
				ShowImage (imgpalace[iThingShow][iModifierShow], iXY[0], iXY[1],
					sShowTile, ascreen, iScale, 1);
			} else {
				ShowImage (imgunk[1], iXY[0], iXY[1], "imgunk[1]",
					ascreen, iScale, 1);
				iInfoC = 1;
			}
		}
	}
	if ((iThingShow == 0x06) || (iThingShow == 0x0F))
	{
		snprintf (arText[0], MAX_TEXT, "E:%i", iModifier + 1);
		DisplayText (iXY[0] + 7, iXY[1] + 151, 11, arText,
			1, font11, color_wh, color_bl, 0);
	}
	if (iSelected == iLocation)
	{
		iShown = 0;
		if (iNoAnim == 0)
		{
			if ((iThingShow == 0x12) && (iModifierShow == 0x00))
			{
				ShowImage (spritechompersel1, iXY[0], iXY[1],
					"spritechompersel1", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x12) && (iModifierShow == 0x80))
			{
				ShowImage (spritechompersel2, iXY[0], iXY[1],
					"spritechompersel2", ascreen, iScale, 1); iShown = 1;
			}
			if ((iThingShow == 0x16) && (iModifierShow == 0x00))
			{
				ShowImage (spriteswordsel, iXY[0], iXY[1],
					"spriteswordsel", ascreen, iScale, 1); iShown = 1;
			}
		}
		if (iShown == 0)
		{
			if (imgselected[iThingShow][iModifierShow] != NULL)
			{
				snprintf (sShowTile, MAX_TEXT, "imgselected[%i][%i]",
					iThingShow, iModifierShow);
				ShowImage (imgselected[iThingShow][iModifierShow], iXY[0], iXY[1],
					sShowTile, ascreen, iScale, 1);
			} else {
				ShowImage (imgunk[2], iXY[0], iXY[1], "imgunk[2]",
					ascreen, iScale, 1);
			}
		}
	}
	if ((iInfo == 1) || (iInfoC == 1))
	{
		snprintf (arText[0], MAX_TEXT, "%02X %02X", iThingShow, iModifierShow);
		DisplayText (iXY[0] + 7, iXY[1] + 151, 11, arText,
			1, font11, color_wh, color_bl, 0);
	}
	if ((iLocation == 100) || (iLocation == 101) || (iLocation == 102))
	{
		ShowImage (imgfadeds, iXY[0], iXY[1], "imgfadeds", ascreen, iScale, 1);
	}
}
/*****************************************************************************/
void LSeekLevel (int iFd, int iLevel)
/*****************************************************************************/
{
	if ((iLevel < 0) || (iLevel > MAX_LEVEL))
	{
		printf ("[FAILED] LSeekLevel() to level: %i!\n", iLevel);
		exit (EXIT_ERROR);
	}
	lseek (iFd, 0x68BE1 + (iLevel * 2308), SEEK_SET);
}
/*****************************************************************************/
void WriteCharByChar (int iFd, unsigned char *sString, int iLength)
/*****************************************************************************/
{
	int iTemp;
	char sToWrite[MAX_TOWRITE + 2];

	for (iTemp = 0; iTemp < iLength; iTemp++)
	{
		snprintf (sToWrite, MAX_TOWRITE, "%c", sString[iTemp]);
		write (iFd, sToWrite, 1);
	}
}
/*****************************************************************************/
void Prev (void)
/*****************************************************************************/
{
	int iToLoad;

	if (iCurLevel != 1)
	{
		iToLoad = iCurLevel - 1;
		if (iToLoad == -1) { iToLoad = 15; }
		LoadLevel (iToLoad);
		ShowScreen (iScreen);
		PlaySound ("wav/level_change.wav");
	}
}
/*****************************************************************************/
void Next (void)
/*****************************************************************************/
{
	int iToLoad;

	if (iCurLevel != 0)
	{
		iToLoad = iCurLevel + 1;
		if (iToLoad == 16) { iToLoad = 0; }
		LoadLevel (iToLoad);
		ShowScreen (iScreen);
		PlaySound ("wav/level_change.wav");
	}
}
/*****************************************************************************/
void LocationToXY (int iLocation, int iXY[4])
/*****************************************************************************/
{
	switch (iLocation)
	{
		/*** above this room ***/
		case -10: iXY[0] = 24+(64*0); iXY[1] = 10+(126*-1); break;
		case -9: iXY[0] = 24+(64*1); iXY[1] = 10+(126*-1); break;
		case -8: iXY[0] = 24+(64*2); iXY[1] = 10+(126*-1); break;
		case -7: iXY[0] = 24+(64*3); iXY[1] = 10+(126*-1); break;
		case -6: iXY[0] = 24+(64*4); iXY[1] = 10+(126*-1); break;
		case -5: iXY[0] = 24+(64*5); iXY[1] = 10+(126*-1); break;
		case -4: iXY[0] = 24+(64*6); iXY[1] = 10+(126*-1); break;
		case -3: iXY[0] = 24+(64*7); iXY[1] = 10+(126*-1); break;
		case -2: iXY[0] = 24+(64*8); iXY[1] = 10+(126*-1); break;
		case -1: iXY[0] = 24+(64*9); iXY[1] = 10+(126*-1); break;
		/*** this room ***/
		case 1: iXY[0] = 24+(64*0); iXY[1] = 10+(126*0); break;
		case 2: iXY[0] = 24+(64*1); iXY[1] = 10+(126*0); break;
		case 3: iXY[0] = 24+(64*2); iXY[1] = 10+(126*0); break;
		case 4: iXY[0] = 24+(64*3); iXY[1] = 10+(126*0); break;
		case 5: iXY[0] = 24+(64*4); iXY[1] = 10+(126*0); break;
		case 6: iXY[0] = 24+(64*5); iXY[1] = 10+(126*0); break;
		case 7: iXY[0] = 24+(64*6); iXY[1] = 10+(126*0); break;
		case 8: iXY[0] = 24+(64*7); iXY[1] = 10+(126*0); break;
		case 9: iXY[0] = 24+(64*8); iXY[1] = 10+(126*0); break;
		case 10: iXY[0] = 24+(64*9); iXY[1] = 10+(126*0); break;
		case 11: iXY[0] = 24+(64*0); iXY[1] = 10+(126*1); break;
		case 12: iXY[0] = 24+(64*1); iXY[1] = 10+(126*1); break;
		case 13: iXY[0] = 24+(64*2); iXY[1] = 10+(126*1); break;
		case 14: iXY[0] = 24+(64*3); iXY[1] = 10+(126*1); break;
		case 15: iXY[0] = 24+(64*4); iXY[1] = 10+(126*1); break;
		case 16: iXY[0] = 24+(64*5); iXY[1] = 10+(126*1); break;
		case 17: iXY[0] = 24+(64*6); iXY[1] = 10+(126*1); break;
		case 18: iXY[0] = 24+(64*7); iXY[1] = 10+(126*1); break;
		case 19: iXY[0] = 24+(64*8); iXY[1] = 10+(126*1); break;
		case 20: iXY[0] = 24+(64*9); iXY[1] = 10+(126*1); break;
		case 21: iXY[0] = 24+(64*0); iXY[1] = 10+(126*2); break;
		case 22: iXY[0] = 24+(64*1); iXY[1] = 10+(126*2); break;
		case 23: iXY[0] = 24+(64*2); iXY[1] = 10+(126*2); break;
		case 24: iXY[0] = 24+(64*3); iXY[1] = 10+(126*2); break;
		case 25: iXY[0] = 24+(64*4); iXY[1] = 10+(126*2); break;
		case 26: iXY[0] = 24+(64*5); iXY[1] = 10+(126*2); break;
		case 27: iXY[0] = 24+(64*6); iXY[1] = 10+(126*2); break;
		case 28: iXY[0] = 24+(64*7); iXY[1] = 10+(126*2); break;
		case 29: iXY[0] = 24+(64*8); iXY[1] = 10+(126*2); break;
		case 30: iXY[0] = 24+(64*9); iXY[1] = 10+(126*2); break;
		/*** under this room ***/
		case 31: iXY[0] = 24+(64*0); iXY[1] = 10+(126*3); break;
		case 32: iXY[0] = 24+(64*1); iXY[1] = 10+(126*3); break;
		case 33: iXY[0] = 24+(64*2); iXY[1] = 10+(126*3); break;
		case 34: iXY[0] = 24+(64*3); iXY[1] = 10+(126*3); break;
		case 35: iXY[0] = 24+(64*4); iXY[1] = 10+(126*3); break;
		case 36: iXY[0] = 24+(64*5); iXY[1] = 10+(126*3); break;
		case 37: iXY[0] = 24+(64*6); iXY[1] = 10+(126*3); break;
		case 38: iXY[0] = 24+(64*7); iXY[1] = 10+(126*3); break;
		case 39: iXY[0] = 24+(64*8); iXY[1] = 10+(126*3); break;
		case 40: iXY[0] = 24+(64*9); iXY[1] = 10+(126*3); break;
		/*** left of this room ***/
		case 100: iXY[0] = 24+(64*-1); iXY[1] = 10+(126*0); break;
		case 101: iXY[0] = 24+(64*-1); iXY[1] = 10+(126*1); break;
		case 102: iXY[0] = 24+(64*-1); iXY[1] = 10+(126*2); break;
		/*** lower left outside ***/
		case 103: iXY[0] = 24+(64*-1); iXY[1] = 10+(126*3); break;
		/*** event target ***/
		case 104: iXY[0] = 64 + 8; iXY[1] = 130 + 8; break;
		/*** custom tile ***/
		case 105: iXY[0] = 82; iXY[1] = 154; break;
		default:
			printf ("[ WARN ] Unknown tile location: %i!\n", iLocation);
			break;
	}
}
/*****************************************************************************/
void ShowMap (void)
/*****************************************************************************/
{
	float fLowest;

	/*** Used for looping. ***/
	int iLoopRoom;

	ShowImage (imgmapgrid, MapGridStartX(), MapGridStartY(), "imgmapgrid",
		mscreen, ZoomGet(), 0);
	WhereToStart();

	/*** Get hover. ***/
	iMapHoverYes = 0;
	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
		{ iDone[iLoopRoom] = 0; }
	SetMapHover ((int)luKidRoom, iStartRoomsX, iStartRoomsY);
	if (iMapHoverYes == 0) { iMapHoverRoom = 0; }

	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
		{ iDone[iLoopRoom] = 0; }
	ShowRoomsMap ((int)luKidRoom, iStartRoomsX, iStartRoomsY);

	ShowImage (imgmap, 0, 0, "imgmap", mscreen, 1, 1);

	/*** Close ***/
	if (iDownAtMap == 1)
	{ /*** down ***/
		ShowImage (imgclose[2], 1189, 809, "imgclose[2]", mscreen, 1, 1);
	} else { /*** up ***/
		ShowImage (imgclose[1], 1189, 809, "imgclose[1]", mscreen, 1, 1);
	}

	/*** Zoom: in ***/
	if (ZoomGet() == MAX_ZOOM)
	{ /*** off ***/
		ShowImage (imgzoominoff, 17, 809, "imgzoominoff", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 2)
		{
			ShowImage (imgzoominon_1, 17, 809, "imgzoominon_1", mscreen, 1, 1);
		} else {
			ShowImage (imgzoominon_0, 17, 809, "imgzoominon_0", mscreen, 1, 1);
		}
	}

	/*** Zoom: out ***/
	if (ZoomGet() == 1)
	{ /*** off ***/
		ShowImage (imgzoomoutoff, 64, 809, "imgzoomoutoff", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 3)
		{
			ShowImage (imgzoomouton_1, 64, 809,
				"imgzoomouton_1", mscreen, 1, 1);
		} else {
			ShowImage (imgzoomouton_0, 64, 809,
				"imgzoomouton_0", mscreen, 1, 1);
		}
	}

	/*** Zoom: 1 ***/
	if (ZoomGet() == DEFAULT_ZOOM)
	{ /*** off ***/
		ShowImage (imgzoom1off, 111, 809, "imgzoom1off", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 4)
		{
			ShowImage (imgzoom1on_1, 111, 809, "imgzoom1on_1", mscreen, 1, 1);
		} else {
			ShowImage (imgzoom1on_0, 111, 809, "imgzoom1on_0", mscreen, 1, 1);
		}
	}

	/*** Zoom: fit ***/
	if (iZoom == 0)
	{ /*** off ***/
		ShowImage (imgzoomfitoff, 158, 809, "imgzoomfitoff", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 5)
		{
			ShowImage (imgzoomfiton_1, 158, 809,
				"imgzoomfiton_1", mscreen, 1, 1);
		} else {
			ShowImage (imgzoomfiton_0, 158, 809,
				"imgzoomfiton_0", mscreen, 1, 1);
		}
	}

	/*** arrow left ***/
	if (MapGridStartX() == 33)
	{
		ShowImage (imgarrowloff, 252, 809, "imgarrowloff", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 6)
		{
			ShowImage (imgarrowlon_1, 252, 809, "imgarrowlon_1", mscreen, 1, 1);
		} else {
			ShowImage (imgarrowlon_0, 252, 809, "imgarrowlon_0", mscreen, 1, 1);
		}
	}

	/*** arrow up ***/
	if (MapGridStartY() == 33)
	{
		ShowImage (imgarrowuoff, 299, 809, "imgarrowuoff", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 7)
		{
			ShowImage (imgarrowuon_1, 299, 809, "imgarrowuon_1", mscreen, 1, 1);
		} else {
			ShowImage (imgarrowuon_0, 299, 809, "imgarrowuon_0", mscreen, 1, 1);
		}
	}

	/*** arrow down ***/
	fLowest = 33 - (MAP_BIG_AREA_H * (ZoomGet() - 1));
	if (MapGridStartY() == fLowest)
	{
		ShowImage (imgarrowdoff, 346, 809, "imgarrowdoff", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 8)
		{
			ShowImage (imgarrowdon_1, 346, 809, "imgarrowdon_1", mscreen, 1, 1);
		} else {
			ShowImage (imgarrowdon_0, 346, 809, "imgarrowdon_0", mscreen, 1, 1);
		}
	}

	/*** arrow right ***/
	fLowest = 33 - (MAP_BIG_AREA_W * (ZoomGet() - 1));
	if (MapGridStartX() == fLowest)
	{
		ShowImage (imgarrowroff, 393, 809, "imgarrowroff", mscreen, 1, 1);
	} else {
		if (iDownAtMap == 9)
		{
			ShowImage (imgarrowron_1, 393, 809, "imgarrowron_1", mscreen, 1, 1);
		} else {
			ShowImage (imgarrowron_0, 393, 809, "imgarrowron_0", mscreen, 1, 1);
		}
	}

	/*** show room numbers ***/
	if (iMapShowNumbers == 1)
	{
		ShowImage (imgsellm, 636, 807, "imgsellm", mscreen, 1, 1);
	}

	/*** refresh screen ***/
	SDL_RenderPresent (mscreen);
}
/*****************************************************************************/
void SetLocation (int iRoom, int iLocation, int iThing, int iModifier)
/*****************************************************************************/
{
	iThingA[iRoom][iLocation] = iThing;
	iModifierA[iRoom][iLocation] = iModifier;

	iLastThing = iThing;
	iLastModifier = iModifier;
}
/*****************************************************************************/
void ChangePosAction (char *sAction)
/*****************************************************************************/
{
	if (strcmp (sAction, "left") == 0)
	{
		do {
			iOnTile--;
			switch (iOnTile)
			{
				case 0: iOnTile = 13; break;
				case 13: iOnTile = 26; break;
				case 26: iOnTile = 39; break;
				case 39: iOnTile = 52; break;
				case 52: iOnTile = 65; break;
				case 65: iOnTile = 78; break;
				case 78: iOnTile = 90; break;
				case 90: iOnTile = 102; break;
			}
		} while (IsDisabled (iOnTile) == 1);
	}

	if (strcmp (sAction, "right") == 0)
	{
		do {
			iOnTile++;
			switch (iOnTile)
			{
				case 14: iOnTile = 1; break;
				case 27: iOnTile = 14; break;
				case 40: iOnTile = 27; break;
				case 53: iOnTile = 40; break;
				case 66: iOnTile = 53; break;
				case 79: iOnTile = 66; break;
				case 91: iOnTile = 79; break;
				case 103: iOnTile = 91; break;
			}
		} while (IsDisabled (iOnTile) == 1);
	}

	if (strcmp (sAction, "up") == 0)
	{
		switch (iOnTile)
		{
			case 1: iOnTile = 91; break;
			case 2: iOnTile = 93; break;
			case 3: iOnTile = 95; break;
			case 4: iOnTile = 97; break;
			case 5: iOnTile = 99; break;
			case 6: iOnTile = 101; break;
			case 7: iOnTile = 72; break;
			case 8: iOnTile = 73; break;
			case 9: iOnTile = 74; break;
			case 10: iOnTile = 75; break;
			case 11: iOnTile = 76; break;
			case 12: iOnTile = 77; break;
			case 13: iOnTile = 78; break;
			case 79: iOnTile = 66; break;
			case 80: iOnTile = 66; break;
			case 81: iOnTile = 67; break;
			case 82: iOnTile = 67; break;
			case 83: iOnTile = 68; break;
			case 84: iOnTile = 68; break;
			case 85: iOnTile = 69; break;
			case 86: iOnTile = 69; break;
			case 87: iOnTile = 70; break;
			case 88: iOnTile = 70; break;
			case 89: iOnTile = 71; break;
			case 90: iOnTile = 71; break;
			case 91: iOnTile = 79; break;
			case 92: iOnTile = 80; break;
			case 93: iOnTile = 81; break;
			case 94: iOnTile = 82; break;
			case 95: iOnTile = 83; break;
			case 96: iOnTile = 84; break;
			case 97: iOnTile = 85; break;
			case 98: iOnTile = 86; break;
			case 99: iOnTile = 87; break;
			case 100: iOnTile = 88; break;
			case 101: iOnTile = 89; break;
			case 102: iOnTile = 90; break;
			default: iOnTile-=13; break;
		}
		while (IsDisabled (iOnTile) == 1) { iOnTile--; }
	}

	if (strcmp (sAction, "down") == 0)
	{
		switch (iOnTile)
		{
			case 66: iOnTile = 79; break;
			case 67: iOnTile = 81; break;
			case 68: iOnTile = 83; break;
			case 69: iOnTile = 85; break;
			case 70: iOnTile = 87; break;
			case 71: iOnTile = 89; break;
			case 72: iOnTile = 7; break;
			case 73: iOnTile = 8; break;
			case 74: iOnTile = 9; break;
			case 75: iOnTile = 10; break;
			case 76: iOnTile = 11; break;
			case 77: iOnTile = 12; break;
			case 78: iOnTile = 13; break;
			case 79: iOnTile = 91; break;
			case 80: iOnTile = 92; break;
			case 81: iOnTile = 93; break;
			case 82: iOnTile = 94; break;
			case 83: iOnTile = 95; break;
			case 84: iOnTile = 96; break;
			case 85: iOnTile = 97; break;
			case 86: iOnTile = 98; break;
			case 87: iOnTile = 99; break;
			case 88: iOnTile = 100; break;
			case 89: iOnTile = 101; break;
			case 90: iOnTile = 102; break;
			case 91: iOnTile = 1; break;
			case 92: iOnTile = 1; break;
			case 93: iOnTile = 2; break;
			case 94: iOnTile = 2; break;
			case 95: iOnTile = 3; break;
			case 96: iOnTile = 3; break;
			case 97: iOnTile = 4; break;
			case 98: iOnTile = 4; break;
			case 99: iOnTile = 5; break;
			case 100: iOnTile = 5; break;
			case 101: iOnTile = 6; break;
			case 102: iOnTile = 6; break;
			default: iOnTile+=13; break;
		}
		while (IsDisabled (iOnTile) == 1) { iOnTile--; }
	}
}
/*****************************************************************************/
void ChangePos (int iLocation)
/*****************************************************************************/
{
	int iChanging;
	SDL_Event event;
	int iXPosOld, iYPosOld;
	int iUseTile;
	int iNowOn;
	int iChangeGuard;
	int iXJoy1, iYJoy1, iXJoy2, iYJoy2;

	/*** Used for looping. ***/
	int iLoopRoom;
	int iLoopTile;
	int iLoopGuard;

	iChanging = 1;
	iGuardSkill = 0;

	ShowChange (iLocation);
	while (iChanging == 1)
	{
		while (SDL_PollEvent (&event))
		{
			if (MapEvents (event) == 0)
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							if (iOnTile != 0)
							{
								UseTile (iOnTile, iLocation, iCurRoom);
								if (iOnTile <= (6 * 13)) { iChanging = 0; }
								iChanged++;
							}
							break;
						case SDL_CONTROLLER_BUTTON_B:
							iChanging = 0; break;
						case SDL_CONTROLLER_BUTTON_X:
							if (iGuardSkill < 11) { iGuardSkill++; }
								else { iGuardSkill = 0; }
							ApplySkillIfNecessary (iSelected);
							PlaySound ("wav/check_box.wav");
							break;
						case SDL_CONTROLLER_BUTTON_Y:
							PlaySound ("wav/screen2or3.wav");
							iChanging = ChangePosCustom (iLocation);
							break;
						case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							ChangePosAction ("left"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							ChangePosAction ("right"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_UP:
							ChangePosAction ("up"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							ChangePosAction ("down"); break;
						case SDL_CONTROLLER_BUTTON_GUIDE:
							/*** Nothing for now. ***/ break;
					}
					ShowChange (iLocation);
					break;
				case SDL_CONTROLLERAXISMOTION: /*** triggers and analog sticks ***/
					iXJoy1 = SDL_JoystickGetAxis (joystick, 0);
					iYJoy1 = SDL_JoystickGetAxis (joystick, 1);
					iXJoy2 = SDL_JoystickGetAxis (joystick, 3);
					iYJoy2 = SDL_JoystickGetAxis (joystick, 4);
					if ((iXJoy1 < -30000) || (iXJoy2 < -30000)) /*** left ***/
					{
						if ((SDL_GetTicks() - joyleft) > 300)
						{
							ChangeEvent (-1, 1);
							joyleft = SDL_GetTicks();
						}
					}
					if ((iXJoy1 > 30000) || (iXJoy2 > 30000)) /*** right ***/
					{
						if ((SDL_GetTicks() - joyright) > 300)
						{
							ChangeEvent (1, 1);
							joyright = SDL_GetTicks();
						}
					}
					if ((iYJoy1 < -30000) || (iYJoy2 < -30000)) /*** up ***/
					{
						if ((SDL_GetTicks() - joyup) > 300)
						{
							ChangeEvent (10, 1);
							joyup = SDL_GetTicks();
						}
					}
					if ((iYJoy1 > 30000) || (iYJoy2 > 30000)) /*** down ***/
					{
						if ((SDL_GetTicks() - joydown) > 300)
						{
							ChangeEvent (-10, 1);
							joydown = SDL_GetTicks();
						}
					}
					ShowChange (iLocation);
					break;
				case SDL_KEYDOWN:
					iChangeGuard = 0;
					switch (event.key.keysym.sym)
					{
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
							if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								if ((iOnTile >= 1) && (iOnTile <= (6 * 13)))
								{
									for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
									{
										for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
											{ UseTile (iOnTile, iLoopTile, iLoopRoom); }
									}
									iChanging = 0;
									iChanged++;
								}
							} else if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								if ((iOnTile >= 1) && (iOnTile <= (6 * 13)))
								{
									for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
										{ UseTile (iOnTile, iLoopTile, iCurRoom); }
									iChanging = 0;
									iChanged++;
								}
							} else if (iOnTile != 0) {
								UseTile (iOnTile, iLocation, iCurRoom);
								if (iOnTile <= (6 * 13)) { iChanging = 0; }
								iChanged++;
							}
							break;
						case SDLK_ESCAPE:
						case SDLK_q:
						case SDLK_c:
							iChanging = 0; break;
						case SDLK_0: case SDLK_KP_0:
							if (iGuardSkill != 0) { iGuardSkill = 0; iChangeGuard = 1; }
							break;
						case SDLK_1: case SDLK_KP_1:
							if (iGuardSkill != 1) { iGuardSkill = 1; iChangeGuard = 1; }
							break;
						case SDLK_2: case SDLK_KP_2:
							if (iGuardSkill != 2) { iGuardSkill = 2; iChangeGuard = 1; }
							break;
						case SDLK_3: case SDLK_KP_3:
							if (iGuardSkill != 3) { iGuardSkill = 3; iChangeGuard = 1; }
							break;
						case SDLK_4: case SDLK_KP_4:
							if (iGuardSkill != 4) { iGuardSkill = 4; iChangeGuard = 1; }
							break;
						case SDLK_5: case SDLK_KP_5:
							if (iGuardSkill != 5) { iGuardSkill = 5; iChangeGuard = 1; }
							break;
						case SDLK_6: case SDLK_KP_6:
							if (iGuardSkill != 6) { iGuardSkill = 6; iChangeGuard = 1; }
							break;
						case SDLK_7: case SDLK_KP_7:
							if (iGuardSkill != 7) { iGuardSkill = 7; iChangeGuard = 1; }
							break;
						case SDLK_8: case SDLK_KP_8:
							if (iGuardSkill != 8) { iGuardSkill = 8; iChangeGuard = 1; }
							break;
						case SDLK_9: case SDLK_KP_9:
							if (iGuardSkill != 9) { iGuardSkill = 9; iChangeGuard = 1; }
							break;
						case SDLK_a:
							if (iGuardSkill != 10) { iGuardSkill = 10; iChangeGuard = 1; }
							break;
						case SDLK_b:
							if (iGuardSkill != 11) { iGuardSkill = 11; iChangeGuard = 1; }
							break;
						case SDLK_LEFT:
							if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								ChangeEvent (-10, 1);
							} else if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT)) {
								ChangeEvent (-1, 1);
							} else {
								ChangePosAction ("left");
							}
							break;
						case SDLK_RIGHT:
							if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								ChangeEvent (10, 1);
							} else if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT)) {
								ChangeEvent (1, 1);
							} else {
								ChangePosAction ("right");
							}
							break;
						case SDLK_UP:
							ChangePosAction ("up");
							break;
						case SDLK_DOWN:
							ChangePosAction ("down");
							break;
						case SDLK_o:
							PlaySound ("wav/screen2or3.wav");
							iChanging = ChangePosCustom (iLocation);
							break;
					}
					if (iChangeGuard == 1)
					{
						ApplySkillIfNecessary (iSelected);
						PlaySound ("wav/check_box.wav");
					}
					ShowChange (iLocation);
					break;
				case SDL_MOUSEMOTION:
					iXPosOld = iXPos;
					iYPosOld = iYPos;
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					if ((iXPosOld == iXPos) && (iYPosOld == iYPos)) { break; }

					if (InArea (503, 419, 503 + 146, 419 + 30) == 1)
						{ iEventHover = 1; } else { iEventHover = 0; }

					iNowOn = OnTile();
					if ((iOnTile != iNowOn) && (iNowOn != 0))
					{
						if (IsDisabled (iNowOn) == 0) { iOnTile = iNowOn; }
					}

					ShowChange (iLocation);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (654, 0, 654 + 36, 0 + 459) == 1) /*** close ***/
							{ iCloseOn = 1; }
						if (InArea (0, 454, 0 + 654, 454 + 5) == 1) /*** custom ***/
							{ iCustomOn = 1; }
					}
					ShowChange (iLocation);
					break;
				case SDL_MOUSEBUTTONUP:
					iCloseOn = 0;
					iCustomOn = 0;

					/*** On tile or living. ***/
					iUseTile = 0;
					if ((InArea (0, 2, 0 + 652, 2 + 386) == 1) ||
						(InArea (0, 386, 0 + 302, 386 + 66) == 1)) { iUseTile = 1; }

					if (event.button.button == 1)
					{
						for (iLoopGuard = 0; iLoopGuard <= 11; iLoopGuard++)
						{
							if (InArea (311 + (iLoopGuard * 15), 427,
								311 + (iLoopGuard * 15) + 14, 427 + 14) == 1)
							{
								if (iGuardSkill != iLoopGuard)
								{
									iGuardSkill = iLoopGuard;
									ApplySkillIfNecessary (iSelected);
									PlaySound ("wav/check_box.wav");
								}
							}
						}

						/*** Changing the event number. ***/
						if (InArea (519, 424, 519 + 13, 424 + 20) == 1)
							{ ChangeEvent (-10, 1); }
						if (InArea (534, 424, 534 + 13, 424 + 20) == 1)
							{ ChangeEvent (-1, 1); }
						if (InArea (604, 424, 604 + 13, 424 + 20) == 1)
							{ ChangeEvent (1, 1); }
						if (InArea (619, 424, 619 + 13, 424 + 20) == 1)
							{ ChangeEvent (10, 1); }

						if (InArea (654, 0, 654 + 36, 0 + 459) == 1) /*** close ***/
							{ iChanging = 0; }

						if (InArea (0, 454, 0 + 654, 454 + 5) == 1) /*** custom ***/
						{
							PlaySound ("wav/screen2or3.wav");
							iChanging = ChangePosCustom (iLocation);
						}

						if (iUseTile == 1)
						{
							UseTile (iOnTile, iLocation, iCurRoom);
							if (iOnTile <= (6 * 13)) { iChanging = 0; }
							iChanged++;
						}
					}

					if (event.button.button == 2)
					{
						if ((iUseTile == 1) && (iOnTile >= 1) && (iOnTile <= (6 * 13)))
						{
							for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
								{ UseTile (iOnTile, iLoopTile, iCurRoom); }
							iChanging = 0;
							iChanged++;
						}
					}

					if (event.button.button == 3)
					{
						if ((iUseTile == 1) && (iOnTile >= 1) && (iOnTile <= (6 * 13)))
						{
							for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
							{
								for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
									{ UseTile (iOnTile, iLoopTile, iLoopRoom); }
							}
							iChanging = 0;
							iChanged++;
						}
					}

					ShowChange (iLocation);
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED: ShowChange (iLocation); break;
						case SDL_WINDOWEVENT_CLOSE: Quit(); break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							iActiveWindowID = iWindowID; break;
					}
					break;
				case SDL_QUIT: Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/ok_close.wav");
}
/*****************************************************************************/
void ShowChange (int iLocation)
/*****************************************************************************/
{
	int iHoverX, iHoverY;
	int iX, iY;

	if (cCurType == 'd')
	{
		ShowImage (imgdungeont, 0, 0, "imgdungeont", ascreen, iScale, 1);
	} else {
		ShowImage (imgpalacet, 0, 0, "imgpalacet", ascreen, iScale, 1);
	}

	/*** close ***/
	if (iCloseOn == 0)
	{ /*** off ***/
		ShowImage (imgclosebig_0, 654, 0, "imgclosebig_0", ascreen, iScale, 1);
	} else { /*** on ***/
		ShowImage (imgclosebig_1, 654, 0, "imgclosebig_1", ascreen, iScale, 1);
	}

	/*** disable some guards ***/
	DisableSome();

	/*** old tile ***/
	iOnTileOld = OnTileOld();
	if (iOnTileOld != 0)
	{
		iIsCustom = 0;
		iHoverX = (((iOnTileOld - 1) % 13) * 50);
		iHoverY = (((iOnTileOld - 1) / 13) * 64) + 2;
		ShowImage (imgborderbl, iHoverX, iHoverY, "imgborderbl",
			ascreen, iScale, 1);
	} else { iIsCustom = 1; }

	/*** custom ***/
	if (iCustomOn == 0)
	{
		if (iIsCustom == 1)
		{ /*** active ***/
			ShowImage (imgcustoma0, 0, 454, "imgcustoma0", ascreen, iScale, 1);
		} else { /*** inactive ***/
			ShowImage (imgcustomi0, 0, 454, "imgcustomi0", ascreen, iScale, 1);
		}
	} else {
		if (iIsCustom == 1)
		{ /*** active ***/
			ShowImage (imgcustoma1, 0, 454, "imgcustoma1", ascreen, iScale, 1);
		} else { /*** inactive ***/
			ShowImage (imgcustomi1, 0, 454, "imgcustomi1", ascreen, iScale, 1);
		}
	}

	/*** kid ***/
	if ((iCurRoom == (int)luKidRoom) && (iLocation == (int)luKidPos))
	{
		if ((int)luKidDir == 0)
		{ /*** right ***/
			ShowImage (imgbordersl, 250, 386, "imgbordersl", ascreen, iScale, 1);
		} else { /*** left ***/
			ShowImage (imgbordersl, 275, 386, "imgbordersl", ascreen, iScale, 1);
		}
	}

	/*** guard ***/
	if (iLocation == sGuardLocations[iCurRoom - 1] + 1)
	{
		iGuardSkill = sGuardSkills[iCurRoom - 1];
		iX = -1; iY = -1;
		switch (iCurGuard)
		{
			case 0: /*** guards ***/
				switch (sGuardColors[iCurRoom - 1] - 1)
				{
					case 0:
						if (sGuardDirections[iCurRoom - 1] == 0) { iX = 50; iY = 418; }
						if (sGuardDirections[iCurRoom - 1] == 255) { iX = 75; iY = 418; }
						break;
					case 1:
						if (sGuardDirections[iCurRoom - 1] == 0) { iX = 100; iY = 386; }
						if (sGuardDirections[iCurRoom - 1] == 255) { iX = 125; iY = 386; }
						break;
					case 2:
						if (sGuardDirections[iCurRoom - 1] == 0) { iX = 100; iY = 418; }
						if (sGuardDirections[iCurRoom - 1] == 255) { iX = 125; iY = 418; }
						break;
					case 3:
						if (sGuardDirections[iCurRoom - 1] == 0) { iX = 150; iY = 386; }
						if (sGuardDirections[iCurRoom - 1] == 255) { iX = 175; iY = 386; }
						break;
					case 4:
						if (sGuardDirections[iCurRoom - 1] == 0) { iX = 150; iY = 418; }
						if (sGuardDirections[iCurRoom - 1] == 255) { iX = 175; iY = 418; }
						break;
					case 5:
						if (sGuardDirections[iCurRoom - 1] == 0) { iX = 200; iY = 386; }
						if (sGuardDirections[iCurRoom - 1] == 255) { iX = 225; iY = 386; }
						break;
					case 6:
						if (sGuardDirections[iCurRoom - 1] == 0) { iX = 200; iY = 418; }
						if (sGuardDirections[iCurRoom - 1] == 255) { iX = 225; iY = 418; }
						break;
					default:
						/*** Do not set iX/iY. ***/
						if (iDebug == 1)
						{
							printf ("[ WARN ] Strange guard color: %i!\n",
								sGuardColors[iCurRoom - 1] - 1);
						}
						break;
				}
				break;
			case 1: /*** fat ***/
				if (sGuardDirections[iCurRoom - 1] == 0) { iX = 0; iY = 418; }
				if (sGuardDirections[iCurRoom - 1] == 255) { iX = 25; iY = 418; }
				break;
			case 2: /*** skeleton ***/
				if (sGuardDirections[iCurRoom - 1] == 0) { iX = 50; iY = 386; }
				if (sGuardDirections[iCurRoom - 1] == 255) { iX = 75; iY = 386; }
				break;
			case 3: /*** Jaffar ***/
				if (sGuardDirections[iCurRoom - 1] == 0) { iX = 250; iY = 418; }
				if (sGuardDirections[iCurRoom - 1] == 255) { iX = 275; iY = 418; }
				break;
			case 4: /*** shadow ***/
				if (sGuardDirections[iCurRoom - 1] == 0) { iX = 0; iY = 386; }
				if (sGuardDirections[iCurRoom - 1] == 255) { iX = 25; iY = 386; }
				break;
		}
		if ((iX != -1) && (iY != -1))
			{ ShowImage (imgbordersl, iX, iY, "imgbordersl", ascreen, iScale, 1); }
	}
	ShowImage (imgsell, 311 + (15 * iGuardSkill), 427, "imgsell",
		ascreen, iScale, 1);

	/*** hover ***/
	if (IsDisabled (iOnTile) == 0)
	{
		if (iOnTile <= (6 * 13))
		{ /*** tile ***/
			iHoverX = (((iOnTile - 1) % 13) * 50);
			iHoverY = (((iOnTile - 1) / 13) * 64) + 2;
			ShowImage (imgborderb, iHoverX, iHoverY, "imgborderb",
				ascreen, iScale, 1);
		} else { /*** living ***/
			iHoverX = ((((iOnTile - 1) - (6 * 13)) % 12) * 25);
			iHoverY = ((((iOnTile - 1) - (6 * 13)) / 12) * 32) + 386;
			ShowImage (imgborders, iHoverX, iHoverY, "imgborders",
				ascreen, iScale, 1);
		}
	}

	/*** event number ***/
	CenterNumber (ascreen, iChangeEvent + 1, 547, 424, color_bl, 0);

	if (iEventHover == 1)
		{ ShowImage (imgeventh, 500, 66, "imgeventh", ascreen, iScale, 1); }

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void UseTile (int iTile, int iLocation, int iRoom)
/*****************************************************************************/
{
	int iToDir, iToColor;

	/*** Make sure the disabled guards can't be used. ***/
	if ((((iTile >= 83) && (iTile <= 88)) ||
		((iTile >= 93) && (iTile <= 100))) && (iCurGuard != 0)) { return; }
	if (((iTile == 91) || (iTile == 92)) && (iCurGuard != 1)) { return; }
	if (((iTile == 81) || (iTile == 82)) && (iCurGuard != 2)) { return; }
	if (((iTile == 101) || (iTile == 102)) && (iCurGuard != 3)) { return; }
	if (((iTile == 79) || (iTile == 80)) && (iCurGuard != 4)) { return; }

	if (iTile == -1) /*** random ***/
	{
		switch (cCurType)
		{
			case 'd':
				do {
					iTile = 1 + (int) (78.0 * rand() / (RAND_MAX + 1.0));
				} while ((iTile == 48) || (iTile == 64) || (iTile == 65) ||
					(iTile == 67) || (iTile == 68) || (iTile == 69) || (iTile == 70));
				break;
			case 'p':
				do {
					iTile = 1 + (int) (78.0 * rand() / (RAND_MAX + 1.0));
				} while (iTile == 62);
				break;
		}
	}

	iToDir = 0; iToColor = -1; /*** To prevent warnings. ***/
	switch (iTile)
	{
		case 1: SetLocation (iRoom, iLocation, 0x00, 0x00); break;
		case 2: SetLocation (iRoom, iLocation, 0x00, 0x01); break;
		case 3: SetLocation (iRoom, iLocation, 0x00, 0x02); break;
		case 4: SetLocation (iRoom, iLocation, 0x00, 0x03); break;
		case 5: SetLocation (iRoom, iLocation, 0x00, 0xFF); break;
		case 6: SetLocation (iRoom, iLocation, 0x01, 0x00); break;
		case 7: SetLocation (iRoom, iLocation, 0x01, 0x01); break;
		case 8: SetLocation (iRoom, iLocation, 0x01, 0x02); break;
		case 9: SetLocation (iRoom, iLocation, 0x01, 0x03); break;
		case 10: SetLocation (iRoom, iLocation, 0x01, 0xFF); break;
		case 11: SetLocation (iRoom, iLocation, 0x02, 0x00); break;
		case 12: SetLocation (iRoom, iLocation, 0x02, 0x01); break;
		case 13: SetLocation (iRoom, iLocation, 0x02, 0x02); break;
		case 14: SetLocation (iRoom, iLocation, 0x02, 0x03); break;
		case 15: SetLocation (iRoom, iLocation, 0x02, 0x04); break;
		case 16: SetLocation (iRoom, iLocation, 0x02, 0x05); break;
		case 17: SetLocation (iRoom, iLocation, 0x02, 0x06); break;
		case 18: SetLocation (iRoom, iLocation, 0x02, 0x07); break;
		case 19: SetLocation (iRoom, iLocation, 0x02, 0x08); break;
		case 20: SetLocation (iRoom, iLocation, 0x02, 0x09); break;
		case 21: SetLocation (iRoom, iLocation, 0x03, 0x00); break;
		case 22: SetLocation (iRoom, iLocation, 0x04, 0x00); break;
		case 23: SetLocation (iRoom, iLocation, 0x04, 0x01); break;
		case 24: SetLocation (iRoom, iLocation, 0x05, 0x00); break;
		case 25: SetLocation (iRoom, iLocation, 0x06, iChangeEvent); break;
		case 26: SetLocation (iRoom, iLocation, 0x07, 0x00); break;
		case 27: SetLocation (iRoom, iLocation, 0x07, 0x01); break;
		case 28: SetLocation (iRoom, iLocation, 0x07, 0x02); break;
		case 29: SetLocation (iRoom, iLocation, 0x07, 0x03); break;
		case 30: SetLocation (iRoom, iLocation, 0x08, 0x00); break;
		case 31: SetLocation (iRoom, iLocation, 0x09, 0x00); break;
		case 32: SetLocation (iRoom, iLocation, 0x0A, 0x00); break;
		case 33: SetLocation (iRoom, iLocation, 0x0A, 0x01); break;
		case 34: SetLocation (iRoom, iLocation, 0x0A, 0x02); break;
		case 35: SetLocation (iRoom, iLocation, 0x0A, 0x03); break;
		case 36: SetLocation (iRoom, iLocation, 0x0A, 0x04); break;
		case 37: SetLocation (iRoom, iLocation, 0x0A, 0x05); break;
		case 38: SetLocation (iRoom, iLocation, 0x0A, 0x06); break;
		case 39: SetLocation (iRoom, iLocation, 0x0B, 0x00); break;
		case 40: SetLocation (iRoom, iLocation, 0x0C, 0x00); break;
		case 41: SetLocation (iRoom, iLocation, 0x0C, 0x01); break;
		case 42: SetLocation (iRoom, iLocation, 0x0C, 0x02); break;
		case 43: SetLocation (iRoom, iLocation, 0x0C, 0x03); break;
		case 44: SetLocation (iRoom, iLocation, 0x0C, 0x04); break;
		case 45: SetLocation (iRoom, iLocation, 0x0C, 0x05); break;
		case 46: SetLocation (iRoom, iLocation, 0x0C, 0x06); break;
		case 47: SetLocation (iRoom, iLocation, 0x0C, 0x07); break;
		case 48: SetLocation (iRoom, iLocation, 0x0D, 0x00); break;
		case 49: SetLocation (iRoom, iLocation, 0x0E, 0x00); break;
		case 50: SetLocation (iRoom, iLocation, 0x0F, iChangeEvent); break;
		case 51: SetLocation (iRoom, iLocation, 0x10, 0x00); break;
		case 52: SetLocation (iRoom, iLocation, 0x11, 0x00); break;
		case 53: SetLocation (iRoom, iLocation, 0x12, 0x00); break;
		case 54: SetLocation (iRoom, iLocation, 0x12, 0x01); break;
		case 55: SetLocation (iRoom, iLocation, 0x12, 0x02); break;
		case 56: SetLocation (iRoom, iLocation, 0x12, 0x03); break;
		case 57: SetLocation (iRoom, iLocation, 0x12, 0x04); break;
		case 58: SetLocation (iRoom, iLocation, 0x12, 0x05); break;
		case 59: SetLocation (iRoom, iLocation, 0x13, 0x00); break;
		case 60: SetLocation (iRoom, iLocation, 0x14, 0x00); break;
		case 61: SetLocation (iRoom, iLocation, 0x14, 0x01); break;
		case 62: SetLocation (iRoom, iLocation, 0x15, 0x00); break;
		case 63: SetLocation (iRoom, iLocation, 0x16, 0x00); break;
		case 64: SetLocation (iRoom, iLocation, 0x17, 0x00); break;
		case 65: SetLocation (iRoom, iLocation, 0x18, 0x00); break;
		case 66: SetLocation (iRoom, iLocation, 0x19, 0x00); break;
		case 67: SetLocation (iRoom, iLocation, 0x1A, 0x00); break;
		case 68: SetLocation (iRoom, iLocation, 0x1B, 0x00); break;
		case 69: SetLocation (iRoom, iLocation, 0x1C, 0x00); break;
		case 70: SetLocation (iRoom, iLocation, 0x1D, 0x00); break;
		case 71: SetLocation (iRoom, iLocation, 0x1E, 0x00); break;
		case 72: SetLocation (iRoom, iLocation, 0x2B, 0x00); break;
		case 73: SetLocation (iRoom, iLocation, 0x12, 0x80); break;
		case 74: SetLocation (iRoom, iLocation, 0x12, 0x81); break;
		case 75: SetLocation (iRoom, iLocation, 0x12, 0x82); break;
		case 76: SetLocation (iRoom, iLocation, 0x12, 0x83); break;
		case 77: SetLocation (iRoom, iLocation, 0x12, 0x84); break;
		case 78: SetLocation (iRoom, iLocation, 0x12, 0x85); break;
		case 79: iToDir = 0; iToColor = -1; break; /*** shadow right ***/
		case 80: iToDir = 255; iToColor = -1; break; /*** shadow left ***/
		case 81: iToDir = 0; iToColor = -1; break; /*** skeleton right ***/
		case 82: iToDir = 255; iToColor = -1; break; /*** skeleton left ***/
		case 83: iToDir = 0; iToColor = 2; break; /*** yellow guard right ***/
		case 84: iToDir = 255; iToColor = 2; break; /*** yellow guard left ***/
		case 85: iToDir = 0; iToColor = 4; break; /*** rose guard right ***/
		case 86: iToDir = 255; iToColor = 4; break; /*** rose guard left ***/
		case 87: iToDir = 0; iToColor = 6; break; /*** blue guard right ***/
		case 88: iToDir = 255; iToColor = 6; break; /*** blue guard left ***/
		case 89: /*** prince right ***/
			if (((int)luKidRoom != iCurRoom) || ((int)luKidPos != iLocation) ||
				(luKidDir != 0))
			{
				luKidRoom = iCurRoom;
				luKidPos = iLocation;
				luKidDir = 0;
				PlaySound ("wav/hum_adj.wav");
			}
			break;
		case 90: /*** prince left ***/
			if (((int)luKidRoom != iCurRoom) || ((int)luKidPos != iLocation) ||
				(luKidDir != 255))
			{
				luKidRoom = iCurRoom;
				luKidPos = iLocation;
				luKidDir = 255;
				PlaySound ("wav/hum_adj.wav");
			}
			break;
		case 91: iToDir = 0; iToColor = -1; break; /*** fat right ***/
		case 92: iToDir = 255; iToColor = -1; break; /*** fat left ***/
		case 93: iToDir = 0; iToColor = 1; break; /*** blush guard right ***/
		case 94: iToDir = 255; iToColor = 1; break; /*** blush guard left ***/
		case 95: iToDir = 0; iToColor = 3; break; /*** rouge guard right ***/
		case 96: iToDir = 255; iToColor = 3; break; /*** rouge guard left ***/
		case 97: iToDir = 0; iToColor = 5; break; /*** dgreen guard right ***/
		case 98: iToDir = 255; iToColor = 5; break; /*** dgreen guard left ***/
		case 99: iToDir = 0; iToColor = 7; break; /*** lgreen guard right ***/
		case 100: iToDir = 255; iToColor = 7; break; /*** lgreen guard left ***/
		case 101: iToDir = 0; iToColor = -1; break; /*** jaffar right ***/
		case 102: iToDir = 255; iToColor = -1; break; /*** jaffar left ***/
	}
	if (((iTile >= 79) && (iTile <= 88)) ||
		((iTile >= 91) && (iTile <= 102)))
	{
		if ((sGuardLocations[iCurRoom - 1] == iLocation - 1) &&
			(sGuardDirections[iCurRoom - 1] == iToDir) &&
			((sGuardColors[iCurRoom - 1] == iToColor) || (iToColor == -1)))
		{ sGuardLocations[iCurRoom - 1] = 30; }
			else { SetGuard (iLocation - 1, iToDir, iGuardSkill, iToColor); }
	}
}
/*****************************************************************************/
int IsDisabled (int iTile)
/*****************************************************************************/
{
	if ((((iTile >= 83) && (iTile <= 88)) ||
		((iTile >= 93) && (iTile <= 100))) && (iCurGuard != 0))
		{ PrIfDe ("[  OK  ] State of guards: disabled\n"); return (1); }
	else if (((iTile == 91) || (iTile == 92)) && (iCurGuard != 1))
		{ PrIfDe ("[  OK  ] State of fat guard: disabled\n"); return (1); }
	else if (((iTile == 81) || (iTile == 82)) && (iCurGuard != 2))
		{ PrIfDe ("[  OK  ] State of skeleton: disabled\n"); return (1); }
	else if (((iTile == 101) || (iTile == 102)) && (iCurGuard != 3))
		{ PrIfDe ("[  OK  ] State of Jaffar: disabled\n"); return (1); }
	else if (((iTile == 79) || (iTile == 80)) && (iCurGuard != 4))
		{ PrIfDe ("[  OK  ] State of shadow: disabled\n"); return (1); }
	else { return (0); }
}
/*****************************************************************************/
int OnTile (void)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iLoopX, iLoopY;

	/*** tiles ***/
	for (iLoopY = 1; iLoopY <= 6; iLoopY++)
	{
		for (iLoopX = 1; iLoopX <= 13; iLoopX++)
		{
			if (InArea (2 + ((iLoopX - 1) * 50),
				4 + ((iLoopY - 1) * 64),
				2 + ((iLoopX - 1) * 50) + 48,
				4 + ((iLoopY - 1) * 64) + 62) == 1)
			{
				return (((iLoopY - 1) * 13) + iLoopX);
			}
		}
	}

	/*** living ***/
	for (iLoopY = 1; iLoopY <= 2; iLoopY++)
	{
		for (iLoopX = 1; iLoopX <= 12; iLoopX++)
		{
			if (InArea (2 + ((iLoopX - 1) * 25),
				388 + ((iLoopY - 1) * 32),
				2 + ((iLoopX - 1) * 25) + 23,
				388 + ((iLoopY - 1) * 32) + 30) == 1)
			{
				return (((iLoopY - 1) * 12) + iLoopX + (6 * 13));
			}
		}
	}

	return (0);
}
/*****************************************************************************/
void SetGuard (int iLoc, int iDirection, int iSkill, int iColor)
/*****************************************************************************/
{
	/* Give 'non-colored' guards a color, to prevent random colors from
	 * appearing (and game crashes from occurring) if we ever switch back
	 * to regular guards.
	 */
	if (iColor == -1) { iColor = 1; }

	sGuardLocations[iCurRoom - 1] = iLoc;
	sGuardDirections[iCurRoom - 1] = iDirection;
	sGuardSkills[iCurRoom - 1] = iSkill;
	sGuardColors[iCurRoom - 1] = iColor;

	PlaySound ("wav/hum_adj.wav");
}
/*****************************************************************************/
void DisableSome (void)
/*****************************************************************************/
{
	if (iCurGuard != 0) /*** guards ***/
	{
		ShowImage (imgnoliving, 50, 418, "imgnoliving", ascreen, iScale, 1);
		ShowImage (imgnoliving, 100, 386, "imgnoliving", ascreen, iScale, 1);
		ShowImage (imgnoliving, 100, 418, "imgnoliving", ascreen, iScale, 1);
		ShowImage (imgnoliving, 150, 386, "imgnoliving", ascreen, iScale, 1);
		ShowImage (imgnoliving, 150, 418, "imgnoliving", ascreen, iScale, 1);
		ShowImage (imgnoliving, 200, 386, "imgnoliving", ascreen, iScale, 1);
		ShowImage (imgnoliving, 200, 418, "imgnoliving", ascreen, iScale, 1);
	}
	if (iCurGuard != 1) /*** fat ***/
		{ ShowImage (imgnoliving, 0, 418, "imgnoliving", ascreen, iScale, 1); }
	if (iCurGuard != 2) /*** skeleton ***/
		{ ShowImage (imgnoliving, 50, 386, "imgnoliving", ascreen, iScale, 1); }
	if (iCurGuard != 3) /*** Jaffar ***/
		{ ShowImage (imgnoliving, 250, 418, "imgnoliving", ascreen, iScale, 1); }
	if (iCurGuard != 4) /*** shadow ***/
		{ ShowImage (imgnoliving, 0, 386, "imgnoliving", ascreen, iScale, 1); }
}
/*****************************************************************************/
int OnTileOld (void)
/*****************************************************************************/
{
	int iOldTile;
	int iTile, iMod;

	iOldTile = 0;
	iTile = iThingA[iCurRoom][iSelected];
	if (iTile > 32) { iTile-=32; }
	iMod = iModifierA[iCurRoom][iSelected];

	/*** row 1 ***/
	if ((iTile == 0x00) && (iMod == 0x00)) { iOldTile = 1; }
	if ((iTile == 0x00) && (iMod == 0x01)) { iOldTile = 2; }
	if ((iTile == 0x00) && (iMod == 0x02)) { iOldTile = 3; }
	if ((iTile == 0x00) && (iMod == 0x03)) { iOldTile = 4; }
	if ((iTile == 0x00) && (iMod == 0xFF)) { iOldTile = 5; }
	if ((iTile == 0x01) && (iMod == 0x00)) { iOldTile = 6; }
	if ((iTile == 0x01) && (iMod == 0x01)) { iOldTile = 7; }
	if ((iTile == 0x01) && (iMod == 0x02)) { iOldTile = 8; }
	if ((iTile == 0x01) && (iMod == 0x03)) { iOldTile = 9; }
	if ((iTile == 0x01) && (iMod == 0xFF)) { iOldTile = 10; }
	if ((iTile == 0x02) && (iMod == 0x00)) { iOldTile = 11; }
	if ((iTile == 0x02) && (iMod == 0x01)) { iOldTile = 12; }
	if ((iTile == 0x02) && (iMod == 0x02)) { iOldTile = 13; }
	/*** row 2 ***/
	if ((iTile == 0x02) && (iMod == 0x03)) { iOldTile = 14; }
	if ((iTile == 0x02) && (iMod == 0x04)) { iOldTile = 15; }
	if ((iTile == 0x02) && (iMod == 0x05)) { iOldTile = 16; }
	if ((iTile == 0x02) && (iMod == 0x06)) { iOldTile = 17; }
	if ((iTile == 0x02) && (iMod == 0x07)) { iOldTile = 18; }
	if ((iTile == 0x02) && (iMod == 0x08)) { iOldTile = 19; }
	if ((iTile == 0x02) && (iMod == 0x09)) { iOldTile = 20; }
	if ((iTile == 0x03) && (iMod == 0x00)) { iOldTile = 21; }
	if ((iTile == 0x04) && ((iMod == 0x00) || (iMod == 0x02))) { iOldTile = 22; }
	if ((iTile == 0x04) && (iMod == 0x01)) { iOldTile = 23; }
	if ((iTile == 0x05) && (iMod == 0x00)) { iOldTile = 24; }
	if (iTile == 0x06) { iOldTile = 25; iChangeEvent = iMod; }
	if ((iTile == 0x07) && (iMod == 0x00)) { iOldTile = 26; }
	/*** row 3 ***/
	if ((iTile == 0x07) && (iMod == 0x01)) { iOldTile = 27; }
	if ((iTile == 0x07) && (iMod == 0x02)) { iOldTile = 28; }
	if ((iTile == 0x07) && (iMod == 0x03)) { iOldTile = 29; }
	if ((iTile == 0x08) && (iMod == 0x00)) { iOldTile = 30; }
	if ((iTile == 0x09) && (iMod == 0x00)) { iOldTile = 31; }
	if ((iTile == 0x0A) && (iMod == 0x00)) { iOldTile = 32; }
	if ((iTile == 0x0A) && (iMod == 0x01)) { iOldTile = 33; }
	if ((iTile == 0x0A) && (iMod == 0x02)) { iOldTile = 34; }
	if ((iTile == 0x0A) && (iMod == 0x03)) { iOldTile = 35; }
	if ((iTile == 0x0A) && (iMod == 0x04)) { iOldTile = 36; }
	if ((iTile == 0x0A) && (iMod == 0x05)) { iOldTile = 37; }
	if ((iTile == 0x0A) && (iMod == 0x06)) { iOldTile = 38; }
	if ((iThingA[iCurRoom][iSelected] == 0x0B) && (iMod == 0x00)) { iOldTile = 39; }
	/*** row 4 ***/
	if ((iTile == 0x0C) && (iMod == 0x00)) { iOldTile = 40; }
	if ((iTile == 0x0C) && (iMod == 0x01)) { iOldTile = 41; }
	if ((iTile == 0x0C) && (iMod == 0x02)) { iOldTile = 42; }
	if ((iTile == 0x0C) && (iMod == 0x03)) { iOldTile = 43; }
	if ((iTile == 0x0C) && (iMod == 0x04)) { iOldTile = 44; }
	if ((iTile == 0x0C) && (iMod == 0x05)) { iOldTile = 45; }
	if ((iTile == 0x0C) && (iMod == 0x06)) { iOldTile = 46; }
	if ((iTile == 0x0C) && (iMod == 0x07)) { iOldTile = 47; }
	if ((iTile == 0x0D) && (iMod == 0x00)) { iOldTile = 48; }
	if ((iTile == 0x0E) && (iMod == 0x00)) { iOldTile = 49; }
	if (iTile == 0x0F) { iOldTile = 50; iChangeEvent = iMod; }
	if ((iTile == 0x10) && (iMod == 0x00)) { iOldTile = 51; }
	if ((iTile == 0x11) && (iMod == 0x00)) { iOldTile = 52; }
	/*** row 5 ***/
	if ((iTile == 0x12) && (iMod == 0x00)) { iOldTile = 53; }
	if ((iTile == 0x12) && (iMod == 0x01)) { iOldTile = 54; }
	if ((iTile == 0x12) && (iMod == 0x02)) { iOldTile = 55; }
	if ((iTile == 0x12) && (iMod == 0x03)) { iOldTile = 56; }
	if ((iTile == 0x12) && (iMod == 0x04)) { iOldTile = 57; }
	if ((iTile == 0x12) && (iMod == 0x05)) { iOldTile = 58; }
	if ((iTile == 0x13) && (iMod == 0x00)) { iOldTile = 59; }
	if ((iTile == 0x14) && (iMod == 0x00)) { iOldTile = 60; }
	if ((iTile == 0x14) && (iMod == 0x01)) { iOldTile = 61; }
	if ((iTile == 0x15) && (iMod == 0x00)) { iOldTile = 62; }
	if ((iTile == 0x16) && (iMod == 0x00)) { iOldTile = 63; }
	if ((iTile == 0x17) && (iMod == 0x00)) { iOldTile = 64; }
	if ((iTile == 0x18) && (iMod == 0x00)) { iOldTile = 65; }
	/*** row 6 ***/
	if ((iTile == 0x19) && (iMod == 0x00)) { iOldTile = 66; }
	if ((iTile == 0x1A) && (iMod == 0x00)) { iOldTile = 67; }
	if ((iTile == 0x1B) && (iMod == 0x00)) { iOldTile = 68; }
	if ((iTile == 0x1C) && (iMod == 0x00)) { iOldTile = 69; }
	if ((iTile == 0x1D) && (iMod == 0x00)) { iOldTile = 70; }
	if ((iTile == 0x1E) && (iMod == 0x00)) { iOldTile = 71; }
	if ((iThingA[iCurRoom][iSelected] == 0x2B) && (iMod == 0x00)) { iOldTile = 72; }
	if ((iTile == 0x12) && (iMod == 0x80)) { iOldTile = 73; }
	if ((iTile == 0x12) && (iMod == 0x81)) { iOldTile = 74; }
	if ((iTile == 0x12) && (iMod == 0x82)) { iOldTile = 75; }
	if ((iTile == 0x12) && (iMod == 0x83)) { iOldTile = 76; }
	if ((iTile == 0x12) && (iMod == 0x84)) { iOldTile = 77; }
	if ((iTile == 0x12) && (iMod == 0x85)) { iOldTile = 78; }

	return (iOldTile);
}
/*****************************************************************************/
void CenterNumber (SDL_Renderer *screen, int iNumber, int iX, int iY,
	SDL_Color fore, int iHex)
/*****************************************************************************/
{
	char sText[MAX_TEXT + 2];

	if (iHex == 0)
	{
		if (iNumber <= 999)
		{
			snprintf (sText, MAX_TEXT, "%i", iNumber);
		} else {
			snprintf (sText, MAX_TEXT, "%s", "X");
		}
	} else {
		snprintf (sText, MAX_TEXT, "%02X", iNumber);
	}
	message = TTF_RenderText_Blended_Wrapped (font20, sText, fore, 0);
	messaget = SDL_CreateTextureFromSurface (screen, message);
	if (iHex == 0)
	{
		if ((iNumber >= -9) && (iNumber <= -1))
		{
			offset.x = iX + 16;
		} else if ((iNumber >= 0) && (iNumber <= 9)) {
			offset.x = iX + 21;
		} else if ((iNumber >= 10) && (iNumber <= 99)) {
			offset.x = iX + 14;
		} else if ((iNumber >= 100) && (iNumber <= 999)) {
			offset.x = iX + 7;
		} else {
			offset.x = iX + 21;
		}
	} else {
		offset.x = iX + 14;
	}
	offset.y = iY - 1;
	offset.w = message->w; offset.h = message->h;
	CustomRenderCopy (messaget, "message", NULL, screen, &offset);
	SDL_DestroyTexture (messaget); SDL_FreeSurface (message);
}
/*****************************************************************************/
void InitRooms (void)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iLoopX;
	int iLoopY;

	for (iLoopX = 0; iLoopX <= ROOMS + 1; iLoopX++)
	{
		for (iLoopY = 0; iLoopY <= ROOMS; iLoopY++)
		{
			iRoomArray[iLoopX][iLoopY] = 0;
		}
	}
}
/*****************************************************************************/
void WhereToStart (void)
/*****************************************************************************/
{
	int iLoopRoom;

	iMinX = 0;
	iMaxX = 0;
	iMinY = 0;
	iMaxY = 0;

	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
	{
		iDone[iLoopRoom] = 0;
	}
	CheckSides ((int)luKidRoom, 0, 0);

	iStartRoomsX = round (12 - (((float)iMinX + (float)iMaxX) / 2));
	iStartRoomsY = round (12 - (((float)iMinY + (float)iMaxY) / 2));
}
/*****************************************************************************/
void ShowRooms (int iRoom, int iX, int iY, int iNext)
/*****************************************************************************/
{
	int iXShow, iYShow;
	char sShowRoom[MAX_TEXT + 2];

	if (iX == 25)
	{
		iXShow = 271;
	} else {
		iXShow = 279 + (iX * 15);
	}
	iYShow = 48 + (iY * 15);

	if (iRoom != -1)
	{
		snprintf (sShowRoom, MAX_TEXT, "imgroom[%i]", iRoom);
		ShowImage (imgroom[iRoom], iXShow, iYShow, sShowRoom,
			ascreen, iScale, 1);
		iRoomArray[iX][iY] = iRoom;
		if (iCurRoom == iRoom) /*** green stripes ***/
			{ ShowImage (imgsrc, iXShow, iYShow, "imgsrc", ascreen, iScale, 1); }
		if ((int)luKidRoom == iRoom) /*** blue border ***/
			{ ShowImage (imgsrs, iXShow, iYShow, "imgsrs", ascreen, iScale, 1); }
		if (iRoom == iMovingRoom) /*** red stripes ***/
			{ ShowImage (imgsrm, iXShow, iYShow, "imgsrm", ascreen, iScale, 1); }
	} else {
		/*** white cross ***/
		ShowImage (imgsrp, iXShow, iYShow, "imgsrp", ascreen, iScale, 1);
	}
	if (iRoom == iMovingRoom)
	{
		iMovingOldX = iX;
		iMovingOldY = iY;
		if (iMovingNewBusy == 0)
		{
			iMovingNewX = iMovingOldX;
			iMovingNewY = iMovingOldY;
			iMovingNewBusy = 1;
		}
	}

	if (iRoom != -1) { iDone[iRoom] = 1; }

	if (iNext == 1)
	{
		if ((iRoomConnections[iRoom][1] != 0) &&
			(iDone[iRoomConnections[iRoom][1]] != 1))
			{ ShowRooms (iRoomConnections[iRoom][1], iX - 1, iY, 1); }

		if ((iRoomConnections[iRoom][2] != 0) &&
			(iDone[iRoomConnections[iRoom][2]] != 1))
			{ ShowRooms (iRoomConnections[iRoom][2], iX + 1, iY, 1); }

		if ((iRoomConnections[iRoom][3] != 0) &&
			(iDone[iRoomConnections[iRoom][3]] != 1))
			{ ShowRooms (iRoomConnections[iRoom][3], iX, iY - 1, 1); }

		if ((iRoomConnections[iRoom][4] != 0) &&
			(iDone[iRoomConnections[iRoom][4]] != 1))
			{ ShowRooms (iRoomConnections[iRoom][4], iX, iY + 1, 1); }
	}
}
/*****************************************************************************/
int MouseSelectAdj (void)
/*****************************************************************************/
{
	int iOnAdj;
	int iAdjBaseX;
	int iAdjBaseY;

	/*** Used for looping. ***/
	int iLoopRoom;

	iOnAdj = 0;

	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
	{
		switch (iLoopRoom)
		{
			case 1: iAdjBaseX = ADJ_BASE_X + (63 * 0);
				iAdjBaseY = ADJ_BASE_Y + (63 * 0); break;
			case 2: iAdjBaseX = ADJ_BASE_X + (63 * 1);
				iAdjBaseY = ADJ_BASE_Y + (63 * 0); break;
			case 3: iAdjBaseX = ADJ_BASE_X + (63 * 2);
				iAdjBaseY = ADJ_BASE_Y + (63 * 0); break;
			case 4: iAdjBaseX = ADJ_BASE_X + (63 * 3);
				iAdjBaseY = ADJ_BASE_Y + (63 * 0); break;
			case 5: iAdjBaseX = ADJ_BASE_X + (63 * 0);
				iAdjBaseY = ADJ_BASE_Y + (63 * 1); break;
			case 6: iAdjBaseX = ADJ_BASE_X + (63 * 1);
				iAdjBaseY = ADJ_BASE_Y + (63 * 1); break;
			case 7: iAdjBaseX = ADJ_BASE_X + (63 * 2);
				iAdjBaseY = ADJ_BASE_Y + (63 * 1); break;
			case 8: iAdjBaseX = ADJ_BASE_X + (63 * 3);
				iAdjBaseY = ADJ_BASE_Y + (63 * 1); break;
			case 9: iAdjBaseX = ADJ_BASE_X + (63 * 0);
				iAdjBaseY = ADJ_BASE_Y + (63 * 2); break;
			case 10: iAdjBaseX = ADJ_BASE_X + (63 * 1);
				iAdjBaseY = ADJ_BASE_Y + (63 * 2); break;
			case 11: iAdjBaseX = ADJ_BASE_X + (63 * 2);
				iAdjBaseY = ADJ_BASE_Y + (63 * 2); break;
			case 12: iAdjBaseX = ADJ_BASE_X + (63 * 3);
				iAdjBaseY = ADJ_BASE_Y + (63 * 2); break;
			case 13: iAdjBaseX = ADJ_BASE_X + (63 * 0);
				iAdjBaseY = ADJ_BASE_Y + (63 * 3); break;
			case 14: iAdjBaseX = ADJ_BASE_X + (63 * 1);
				iAdjBaseY = ADJ_BASE_Y + (63 * 3); break;
			case 15: iAdjBaseX = ADJ_BASE_X + (63 * 2);
				iAdjBaseY = ADJ_BASE_Y + (63 * 3); break;
			case 16: iAdjBaseX = ADJ_BASE_X + (63 * 3);
				iAdjBaseY = ADJ_BASE_Y + (63 * 3); break;
			case 17: iAdjBaseX = ADJ_BASE_X + (63 * 0);
				iAdjBaseY = ADJ_BASE_Y + (63 * 4); break;
			case 18: iAdjBaseX = ADJ_BASE_X + (63 * 1);
				iAdjBaseY = ADJ_BASE_Y + (63 * 4); break;
			case 19: iAdjBaseX = ADJ_BASE_X + (63 * 2);
				iAdjBaseY = ADJ_BASE_Y + (63 * 4); break;
			case 20: iAdjBaseX = ADJ_BASE_X + (63 * 3);
				iAdjBaseY = ADJ_BASE_Y + (63 * 4); break;
			case 21: iAdjBaseX = ADJ_BASE_X + (63 * 0);
				iAdjBaseY = ADJ_BASE_Y + (63 * 5); break;
			case 22: iAdjBaseX = ADJ_BASE_X + (63 * 1);
				iAdjBaseY = ADJ_BASE_Y + (63 * 5); break;
			case 23: iAdjBaseX = ADJ_BASE_X + (63 * 2);
				iAdjBaseY = ADJ_BASE_Y + (63 * 5); break;
			case 24: iAdjBaseX = ADJ_BASE_X + (63 * 3);
				iAdjBaseY = ADJ_BASE_Y + (63 * 5); break;
			default:
				printf ("[FAILED] iLoopRoom is not in the 1-24 range!\n");
				exit (EXIT_ERROR);
		}
		if (InArea (iAdjBaseX + 1, iAdjBaseY + 16,
			iAdjBaseX + 15, iAdjBaseY + 30) == 1)
		{
			iChangingBrokenRoom = iLoopRoom;
			iChangingBrokenSide = 1; /*** left ***/
			iOnAdj = 1;
		}
		if (InArea (iAdjBaseX + 31, iAdjBaseY + 16,
			iAdjBaseX + 45, iAdjBaseY + 30) == 1)
		{
			iChangingBrokenRoom = iLoopRoom;
			iChangingBrokenSide = 2; /*** right ***/
			iOnAdj = 1;
		}
		if (InArea (iAdjBaseX + 16, iAdjBaseY + 1,
			iAdjBaseX + 30, iAdjBaseY + 14) == 1)
		{
			iChangingBrokenRoom = iLoopRoom;
			iChangingBrokenSide = 3; /*** up ***/
			iOnAdj = 1;
		}
		if (InArea (iAdjBaseX + 16, iAdjBaseY + 31,
			iAdjBaseX + 30, iAdjBaseY + 45) == 1)
		{
			iChangingBrokenRoom = iLoopRoom;
			iChangingBrokenSide = 4; /*** down ***/
			iOnAdj = 1;
		}
	}

	return (iOnAdj);
}
/*****************************************************************************/
void RemoveOldRoom (void)
/*****************************************************************************/
{
	iRoomArray[iMovingOldX][iMovingOldY] = 0;

	/* Change the links of the rooms around
	 * the removed room.
	 */

	/*** left of removed ***/
	if ((iMovingOldX >= 2) && (iMovingOldX <= 24))
	{
		if (iRoomArray[iMovingOldX - 1][iMovingOldY] != 0)
		{
			iRoomConnections[iRoomArray[iMovingOldX - 1]
				[iMovingOldY]][2] = 0; /*** remove right ***/
		}
	}

	/*** right of removed ***/
	if ((iMovingOldX >= 1) && (iMovingOldX <= 23))
	{
		if (iRoomArray[iMovingOldX + 1][iMovingOldY] != 0)
		{
			iRoomConnections[iRoomArray[iMovingOldX + 1]
				[iMovingOldY]][1] = 0; /*** remove left ***/
		}
	}

	/*** above removed ***/
	if ((iMovingOldY >= 2) && (iMovingOldY <= 24))
	{
		if (iRoomArray[iMovingOldX][iMovingOldY - 1] != 0)
		{
			iRoomConnections[iRoomArray[iMovingOldX]
				[iMovingOldY - 1]][4] = 0; /*** remove below ***/
		}
	}

	/*** below removed ***/
	if ((iMovingOldY >= 1) && (iMovingOldY <= 23))
	{
		if (iRoomArray[iMovingOldX][iMovingOldY + 1] != 0)
		{
			iRoomConnections[iRoomArray[iMovingOldX]
				[iMovingOldY + 1]][3] = 0; /*** remove above ***/
		}
	}
}
/*****************************************************************************/
void AddNewRoom (int iX, int iY, int iRoom)
/*****************************************************************************/
{
	iRoomArray[iX][iY] = iRoom;

	/* Change the links of the rooms around
	 * the new room and the room itself.
	 */

	iRoomConnections[iRoom][1] = 0;
	iRoomConnections[iRoom][2] = 0;
	iRoomConnections[iRoom][3] = 0;
	iRoomConnections[iRoom][4] = 0;
	if ((iX >= 2) && (iX <= 24)) /*** left of added ***/
	{
		if (iRoomArray[iX - 1][iY] != 0)
		{
			iRoomConnections[iRoomArray[iX - 1]
				[iY]][2] = iRoom; /*** add room right ***/
			iRoomConnections[iRoom][1] = iRoomArray[iX - 1][iY];
		}
	}
	if ((iX >= 1) && (iX <= 23)) /*** right of added ***/
	{
		if (iRoomArray[iX + 1][iY] != 0)
		{
			iRoomConnections[iRoomArray[iX + 1]
				[iY]][1] = iRoom; /*** add room left ***/
			iRoomConnections[iRoom][2] = iRoomArray[iX + 1][iY];
		}
	}
	if ((iY >= 2) && (iY <= 24)) /*** above added ***/
	{
		if (iRoomArray[iX][iY - 1] != 0)
		{
			iRoomConnections[iRoomArray[iX]
				[iY - 1]][4] = iRoom; /*** add room below ***/
			iRoomConnections[iRoom][3] = iRoomArray[iX][iY - 1];
		}
	}
	if ((iY >= 1) && (iY <= 23)) /*** below added ***/
	{
		if (iRoomArray[iX][iY + 1] != 0)
		{
			iRoomConnections[iRoomArray[iX]
				[iY + 1]][3] = iRoom; /*** add room above ***/
			iRoomConnections[iRoom][4] = iRoomArray[iX][iY + 1];
		}
	}
	PlaySound ("wav/move_room.wav");
}
/*****************************************************************************/
void MapShow (void)
/*****************************************************************************/
{
	if (iMapOpen != 1)
	{
		/* On X11, this does not work with SDL versions below 2.0.6.
		 * https://bugzilla.libsdl.org/show_bug.cgi?id=2818
		 */
		SDL_ShowWindow (windowmap);
		iMapOpen = 1;
	}
	/*** SDL_RaiseWindow (window); ***/
	PlaySound ("wav/screen2or3.wav");
}
/*****************************************************************************/
void LinkPlus (void)
/*****************************************************************************/
{
	int iCurrent, iNew;

	iCurrent = iRoomConnections[iChangingBrokenRoom][iChangingBrokenSide];
	if (iCurrent == ROOMS) { iNew = 0; } else { iNew = iCurrent + 1; }
	iRoomConnections[iChangingBrokenRoom][iChangingBrokenSide] = iNew;
	iChanged++;
	iBrokenRoomLinks = BrokenRoomLinks (0);
	PlaySound ("wav/hum_adj.wav");
}
/*****************************************************************************/
void LinkMinus (void)
/*****************************************************************************/
{
	int iCurrent, iNew;

	iCurrent = iRoomConnections[iChangingBrokenRoom][iChangingBrokenSide];
	if (iCurrent == 0) { iNew = ROOMS; } else { iNew = iCurrent - 1; }
	iRoomConnections[iChangingBrokenRoom][iChangingBrokenSide] = iNew;
	iChanged++;
	iBrokenRoomLinks = BrokenRoomLinks (0);
	PlaySound ("wav/hum_adj.wav");
}
/*****************************************************************************/
void ClearRoom (void)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iLoopTile;

	for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{ SetLocation (iCurRoom, iLoopTile, 0, 0); }
	sGuardLocations[iCurRoom - 1] = 30;
	PlaySound ("wav/ok_close.wav");
	iChanged++;
}
/*****************************************************************************/
float ZoomGet (void)
/*****************************************************************************/
{
	float fMinXZoom, fMinYZoom, fZoom;

	if (iZoom == 0)
	{
		fMinXZoom = 24 / (((float)iMaxX - (float)iMinX) + 1);
		fMinYZoom = 24 / (((float)iMaxY - (float)iMinY) + 1);
		fZoom = fMinXZoom;
		if (fMinYZoom < fMinXZoom) { fZoom = fMinYZoom; }
	} else {
		fZoom = iZoom;
	}

	return (fZoom);
}
/*****************************************************************************/
int MapGridStartX (void)
/*****************************************************************************/
{
	float fReturn;
	float fLowest;

	fReturn = 33 - ((MAP_BIG_AREA_W / 2) * (ZoomGet() - 1));
	fReturn += (float)iXPosMapMoveOffset;
	if (fReturn > 33) { fReturn = 33; }
	fLowest = 33 - (MAP_BIG_AREA_W * (ZoomGet() - 1));
	if (fReturn < fLowest) { fReturn = fLowest; }

	return (round (fReturn));
}
/*****************************************************************************/
int MapGridStartY (void)
/*****************************************************************************/
{
	float fReturn;
	float fLowest;

	fReturn = 33 - ((MAP_BIG_AREA_H / 2) * (ZoomGet() - 1));
	fReturn += (float)iYPosMapMoveOffset;
	if (fReturn > 33) { fReturn = 33; }
	fLowest = 33 - (MAP_BIG_AREA_H * (ZoomGet() - 1));
	if (fReturn < fLowest) { fReturn = fLowest; }

	return (round (fReturn));
}
/*****************************************************************************/
void SetMapHover (int iRoom, int iX, int iY)
/*****************************************************************************/
{
	float fOffsetX, fOffsetY;
	float fRoomStartX, fRoomStartY;

	/*** Used for looping. ***/
	int iLoopTile;

	fRoomStartX = MapGridStartX() + ZoomGet() + ((iX - 1) * (51 * ZoomGet()));
	fRoomStartY = MapGridStartY() + ZoomGet() + ((iY - 1) * (31 * ZoomGet()));

	for (iLoopTile = 1; iLoopTile <= 30; iLoopTile++)
	{
		if ((iLoopTile >= 1) && (iLoopTile <= 10))
		{
			fOffsetX = (iLoopTile - 1) * 5;
			fOffsetY = 0;
		}
		else if ((iLoopTile >= 11) && (iLoopTile <= 20))
		{
			fOffsetX = (iLoopTile - 11) * 5;
			fOffsetY = 10;
		}
		else if ((iLoopTile >= 21) && (iLoopTile <= 30))
		{
			fOffsetX = (iLoopTile - 21) * 5;
			fOffsetY = 20;
		}
		fOffsetX = fOffsetX * ZoomGet();
		fOffsetY = fOffsetY * ZoomGet();

		/*** hover tile ***/
		if ((iXPosMap >= fRoomStartX + fOffsetX) &&
			(iXPosMap < fRoomStartX + fOffsetX + (5 * ZoomGet())) &&
			(iYPosMap >= fRoomStartY + fOffsetY) &&
			(iYPosMap < fRoomStartY + fOffsetY + (10 * ZoomGet())))
		{
			iMapHoverTile = iLoopTile;
		}
	}

	/*** hover room ***/
	if ((iXPosMap >= fRoomStartX) &&
		(iXPosMap <= fRoomStartX + (50 * ZoomGet())) &&
		(iYPosMap >= fRoomStartY) &&
		(iYPosMap <= fRoomStartY + (30 * ZoomGet())))
	{
		iMapHoverRoom = iRoom;
		iMapHoverYes = 1;
	}

	iDone[iRoom] = 1;

	if ((iRoomConnections[iRoom][1] != 0) &&
		(iDone[iRoomConnections[iRoom][1]] != 1))
		{ SetMapHover (iRoomConnections[iRoom][1], iX - 1, iY); }

	if ((iRoomConnections[iRoom][2] != 0) &&
		(iDone[iRoomConnections[iRoom][2]] != 1))
		{ SetMapHover (iRoomConnections[iRoom][2], iX + 1, iY); }

	if ((iRoomConnections[iRoom][3] != 0) &&
		(iDone[iRoomConnections[iRoom][3]] != 1))
		{ SetMapHover (iRoomConnections[iRoom][3], iX, iY - 1); }

	if ((iRoomConnections[iRoom][4] != 0) &&
		(iDone[iRoomConnections[iRoom][4]] != 1))
		{ SetMapHover (iRoomConnections[iRoom][4], iX, iY + 1); }
}
/*****************************************************************************/
void ShowRoomsMap (int iRoom, int iX, int iY)
/*****************************************************************************/
{
	char sInfo[MAX_TEXT + 2];
	int iThing, iMod;
	float fOffsetX, fOffsetY;
	float fRoomStartX, fRoomStartY;
	char arText[1 + 2][MAX_TEXT + 2];

	/*** Used for looping. ***/
	int iLoopTile;

	fRoomStartX = MapGridStartX() + ZoomGet() + ((iX - 1) * (51 * ZoomGet()));
	fRoomStartY = MapGridStartY() + ZoomGet() + ((iY - 1) * (31 * ZoomGet()));

	for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
	{
		if ((iLoopTile >= 1) && (iLoopTile <= 10))
		{
			fOffsetX = (iLoopTile - 1) * 5;
			fOffsetY = 0;
		}
		else if ((iLoopTile >= 11) && (iLoopTile <= 20))
		{
			fOffsetX = (iLoopTile - 11) * 5;
			fOffsetY = 10;
		}
		else if ((iLoopTile >= 21) && (iLoopTile <= 30))
		{
			fOffsetX = (iLoopTile - 21) * 5;
			fOffsetY = 20;
		}
		fOffsetX = fOffsetX * ZoomGet();
		fOffsetY = fOffsetY * ZoomGet();

		iThing = iThingA[iRoom][iLoopTile];
		if (iThing > 31) { iThing-=32; }
		iMod = iModifierA[iRoom][iLoopTile];
		switch (iThing)
		{
			case 0: if (iMod != 3) { iMod = 0; } break;
			case 1: iMod = 0; break;
			case 2: iMod = 0; break;
			case 3: iMod = 0; break;
			case 4: if ((iMod != 0) && (iMod != 1)) { iMod = 0; } break;
			case 5: iMod = 0; break;
			case 6: iMod = 0; break;
			case 7: iMod = 0; break;
			case 8: iMod = 0; break;
			case 9: iMod = 0; break;
			case 10: if (iMod > 6) { iMod = 0; } break;
			case 11: iMod = 0; break;
			case 12: iMod = 0; break;
			case 13: iMod = 0; break;
			case 14: iMod = 0; break;
			case 15: iMod = 0; break;
			case 16: iMod = 0; break;
			case 17: iMod = 0; break;
			case 18: if (iMod != 2) { iMod = 0; } break; /*** chomper ***/
			case 19: iMod = 0; break;
			case 20: iMod = 0; break;
			case 21: iMod = 0; break;
			case 22: iMod = 0; break;
			case 23: iMod = 0; break;
			case 24: iMod = 0; break;
			case 25: iMod = 0; break;
			case 26: iMod = 0; break;
			case 27: iMod = 0; break;
			case 28: iMod = 0; break;
			case 29: iMod = 0; break;
			case 30: iMod = 0; break;
		}
		snprintf (sInfo, MAX_TEXT, "imgmini[%i][%i]", iThing, iMod);
		ShowImage (imgmini[iThing][iMod],
			fRoomStartX + fOffsetX,
			fRoomStartY + fOffsetY,
			sInfo, mscreen, ZoomGet(), 0);

		/*** guard ***/
		if (sGuardLocations[iRoom - 1] == iLoopTile - 1)
		{
			ShowImage (imgminiguard, fRoomStartX + fOffsetX,
				fRoomStartY + fOffsetY, "imgminiguard", mscreen, ZoomGet(), 0);
		}

		/*** prince ***/
		if (((int)luKidRoom == iRoom) && ((int)luKidPos == iLoopTile))
		{
			ShowImage (imgminiprince, fRoomStartX + fOffsetX,
				fRoomStartY + fOffsetY, "imgminiprince", mscreen, ZoomGet(), 0);
		}

		/*** hover tile (green) ***/
		if ((iXPosMap >= fRoomStartX + fOffsetX) &&
			(iXPosMap < fRoomStartX + fOffsetX + (5 * ZoomGet())) &&
			(iYPosMap >= fRoomStartY + fOffsetY) &&
			(iYPosMap < fRoomStartY + fOffsetY + (10 * ZoomGet())))
		{
			ShowImage (imgminihover, fRoomStartX + fOffsetX,
				fRoomStartY + fOffsetY, "imgminihover", mscreen, ZoomGet(), 0);
		}

		/*** related tile (yellow) ***/
		if (RelatedToHover (iRoom, iLoopTile) == 1)
		{
			ShowImage (imgminirelated, fRoomStartX + fOffsetX,
				fRoomStartY + fOffsetY, "imgminirelated", mscreen, ZoomGet(), 0);
		}
	}

	/*** active room (green border) ***/
	if (iCurRoom == iRoom)
	{
		ShowImage (imgbmrooma, fRoomStartX - ZoomGet(),
			fRoomStartY - ZoomGet(), "imgbmrooma", mscreen, ZoomGet(), 0);
	}

	/*** hover room (bright green border) ***/
	if ((iXPosMap >= fRoomStartX) &&
		(iXPosMap <= fRoomStartX + (50 * ZoomGet())) &&
		(iYPosMap >= fRoomStartY) &&
		(iYPosMap <= fRoomStartY + (30 * ZoomGet())))
	{
		ShowImage (imgbmroomh, fRoomStartX - ZoomGet(),
			fRoomStartY - ZoomGet(), "imgbmroomh", mscreen, ZoomGet(), 0);
	}

	/*** room number ***/
	if (iMapShowNumbers == 1)
	{
		snprintf (arText[0], MAX_TEXT, "%i", iRoom);
		DisplayText (fRoomStartX, fRoomStartY, 11, arText,
			1, font11, color_wh, color_bl, 1);
	}

	iDone[iRoom] = 1;

	if ((iRoomConnections[iRoom][1] != 0) &&
		(iDone[iRoomConnections[iRoom][1]] != 1))
		{ ShowRoomsMap (iRoomConnections[iRoom][1], iX - 1, iY); }

	if ((iRoomConnections[iRoom][2] != 0) &&
		(iDone[iRoomConnections[iRoom][2]] != 1))
		{ ShowRoomsMap (iRoomConnections[iRoom][2], iX + 1, iY); }

	if ((iRoomConnections[iRoom][3] != 0) &&
		(iDone[iRoomConnections[iRoom][3]] != 1))
		{ ShowRoomsMap (iRoomConnections[iRoom][3], iX, iY - 1); }

	if ((iRoomConnections[iRoom][4] != 0) &&
		(iDone[iRoomConnections[iRoom][4]] != 1))
		{ ShowRoomsMap (iRoomConnections[iRoom][4], iX, iY + 1); }
}
/*****************************************************************************/
int RelatedToHover (int iRoom, int iTile)
/*****************************************************************************/
{
	int iRelated;
	int iHoverThing, iTileThing;
	int iHoverModifier, iTileModifier;
	int iStartEvent;

	iRelated = 0;

	/*** If the hover is a button... ***/
	iHoverThing = iThingA[iMapHoverRoom][iMapHoverTile];
	iHoverModifier = iModifierA[iMapHoverRoom][iMapHoverTile];
	if ((iHoverThing == 6) || (iHoverThing == 38) ||
		(iHoverThing == 15) || (iHoverThing == 47))
	{
		iStartEvent = iHoverModifier;
		do {
			iStartEvent++;
			if ((EventInfo (iStartEvent - 1, 1) == iRoom) &&
				(EventInfo (iStartEvent - 1, 2) == iTile))
			{
				iRelated = 1;
			}
		} while ((EventInfo (iStartEvent - 1, 3) == 1) && (iStartEvent != 256));
	}
	/*** If the hover is a special blue potion... ***/
	if (((iHoverThing == 10) || (iHoverThing == 42)) && (iHoverModifier == 6))
	{
		if ((iRoom == 8) && (iTile == 1)) { iRelated = 1; }
	}

	/*** If this tile is a button, check whether it triggers the hover. ***/
	iTileThing = iThingA[iRoom][iTile];
	iTileModifier = iModifierA[iRoom][iTile];
	if ((iTileThing == 6) || (iTileThing == 38) ||
		(iTileThing == 15) || (iTileThing == 47))
	{
		iStartEvent = iTileModifier;
		do {
			iStartEvent++;
			if ((EventInfo (iStartEvent - 1, 1) == iMapHoverRoom) &&
				(EventInfo (iStartEvent - 1, 2) == iMapHoverTile))
			{
				iRelated = 1;
			}
		} while ((EventInfo (iStartEvent - 1, 3) == 1) && (iStartEvent != 256));
	}
	/* If this tile is a special blue potion,
	 * check whether the hover is in room 8 upper left.
	 */
	if (((iTileThing == 10) || (iTileThing == 42)) && (iTileModifier == 6))
	{
		if ((iMapHoverRoom == 8) && (iMapHoverTile == 1)) { iRelated = 1; }
	}

	return (iRelated);
}
/*****************************************************************************/
void MapAction (char *sAction)
/*****************************************************************************/
{
	float fLowest;

	if (strcmp (sAction, "left") == 0)
	{
		if (MapGridStartX() < 33)
		{
			iXPosMapMoveOffset += (51 * ZoomGet());
			PlaySound ("wav/screen2or3.wav");
		}
	}

	if (strcmp (sAction, "up") == 0)
	{
		if (MapGridStartY() < 33)
		{
			iYPosMapMoveOffset += (31 * ZoomGet());
			PlaySound ("wav/screen2or3.wav");
		}
	}

	if (strcmp (sAction, "down") == 0)
	{
		fLowest = 33 - (MAP_BIG_AREA_H * (ZoomGet() - 1));
		if (MapGridStartY() > fLowest)
		{
			iYPosMapMoveOffset -= (31 * ZoomGet());
			PlaySound ("wav/screen2or3.wav");
		}
	}

	if (strcmp (sAction, "right") == 0)
	{
		fLowest = 33 - (MAP_BIG_AREA_W * (ZoomGet() - 1));
		if (MapGridStartX() > fLowest)
		{
			iXPosMapMoveOffset -= (51 * ZoomGet());
			PlaySound ("wav/screen2or3.wav");
		}
	}

	if (strcmp (sAction, "in") == 0)
	{
		if (ZoomGet() < MAX_ZOOM)
		{
			ZoomIncrease();
			/*** Back to a centered map. ***/
			iXPosMapMoveOffset = 0;
			iYPosMapMoveOffset = 0;
			PlaySound ("wav/screen2or3.wav");
		}
	}

	if (strcmp (sAction, "out") == 0)
	{
		if (ZoomGet() > 1)
		{
			ZoomDecrease();
			/*** Back to a centered map. ***/
			iXPosMapMoveOffset = 0;
			iYPosMapMoveOffset = 0;
			PlaySound ("wav/screen2or3.wav");
		}
	}
}
/*****************************************************************************/
void MapControllerDown (SDL_Event event)
/*****************************************************************************/
{
	if (event.cbutton.button != 0) { } /*** To prevent warnings. ***/

	/*** Nothing for now. ***/
}
/*****************************************************************************/
void MapControllerUp (SDL_Event event)
/*****************************************************************************/
{
	switch (event.cbutton.button)
	{
		case SDL_CONTROLLER_BUTTON_B:
			MapHide();
			break;
		case SDL_CONTROLLER_BUTTON_GUIDE:
			if (iMapShowNumbers == 0)
				{ iMapShowNumbers = 1; }
					else { iMapShowNumbers = 0; }
			PlaySound ("wav/check_box.wav");
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: MapAction ("out"); break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: MapAction ("in"); break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: MapAction ("left"); break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP: MapAction ("up"); break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: MapAction ("down"); break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: MapAction ("right"); break;
	}
}
/*****************************************************************************/
void MapControllerMotion (SDL_Event event)
/*****************************************************************************/
{
	if (event.caxis.axis != 0) { } /*** To prevent warnings. ***/

	/*** Nothing for now. ***/
}
/*****************************************************************************/
void MapButtonDown (SDL_Event event)
/*****************************************************************************/
{
	float fLowest;

	if (event.button.button == 1)
	{
		if (InAreaMap (1189, 809, 1189 + 85, 809 + 32) == 1) /*** Close ***/
			{ iDownAtMap = 1; }
		if (InAreaMap (17, 809, 17 + 32, 809 + 32) == 1) /*** Zoom: in ***/
			{ if (ZoomGet() < MAX_ZOOM) { iDownAtMap = 2; } }
		if (InAreaMap (64, 809, 64 + 32, 809 + 32) == 1) /*** Zoom: out ***/
			{ if (ZoomGet() > 1) { iDownAtMap = 3; } }
		if (InAreaMap (111, 809, 111 + 32, 809 + 32) == 1) /*** Zoom: 1 ***/
			{ if (ZoomGet() != DEFAULT_ZOOM) { iDownAtMap = 4; } }
		if (InAreaMap (158, 809, 158 + 32, 809 + 32) == 1) /*** Zoom: fit ***/
			{ if (iZoom != 0) { iDownAtMap = 5; } }
		if (InAreaMap (252, 809, 252 + 32, 809 + 32) == 1) /*** arrow left ***/
		{
			if (MapGridStartX() < 33)
			{
				iDownAtMap = 6;
			}
		}
		if (InAreaMap (299, 809, 299 + 32, 809 + 32) == 1) /*** arrow up ***/
		{
			if (MapGridStartY() < 33)
			{
				iDownAtMap = 7;
			}
		}
		if (InAreaMap (346, 809, 346 + 32, 809 + 32) == 1) /*** arrow down ***/
		{
			fLowest = 33 - (MAP_BIG_AREA_H * (ZoomGet() - 1));
			if (MapGridStartY() > fLowest)
			{
				iDownAtMap = 8;
			}
		}
		if (InAreaMap (393, 809, 393 + 32, 809 + 32) == 1) /*** arrow right ***/
		{
			fLowest = 33 - (MAP_BIG_AREA_W * (ZoomGet() - 1));
			if (MapGridStartX() > fLowest)
			{
				iDownAtMap = 9;
			}
		}
		if (InAreaMap (33, 33, 33 + MAP_BIG_AREA_W,
			33 + MAP_BIG_AREA_H) == 1) /*** big area ***/
		{
			/*** Start map movement. ***/
			if (iMovingMap == 0)
			{
				iXPosMapMoveStart = iXPosMap;
				iYPosMapMoveStart = iYPosMap;
				iMovingMap = 1;

				iMapMoved = 0;
			}
		}
	}
	ShowMap();
}
/*****************************************************************************/
void MapButtonUp (SDL_Event event)
/*****************************************************************************/
{
	iDownAtMap = 0;

	/*** Stop map movement. ***/
	if (iMovingMap == 1)
	{
		iMovingMap = 0;
		SDL_SetCursor (curArrow);
		/*** No sound here. ***/
	}

	if (event.button.button == 1)
	{
		if (InAreaMap (1189, 809, 1189 + 85, 809 + 32) == 1) /*** Close ***/
			{ MapHide(); }
		if (InAreaMap (17, 809, 17 + 32, 809 + 32) == 1) /*** Zoom: in ***/
			{ MapAction ("in"); }
		if (InAreaMap (64, 809, 64 + 32, 809 + 32) == 1) /*** Zoom: out ***/
			{ MapAction ("out"); }
		if (InAreaMap (111, 809, 111 + 32, 809 + 32) == 1) /*** Zoom: 1 ***/
		{
			if (ZoomGet() != DEFAULT_ZOOM)
			{
				iZoom = DEFAULT_ZOOM;
				/*** Back to a centered map. ***/
				iXPosMapMoveOffset = 0;
				iYPosMapMoveOffset = 0;
				PlaySound ("wav/screen2or3.wav");
			}
		}
		if (InAreaMap (158, 809, 158 + 32, 809 + 32) == 1) /*** Zoom: fit ***/
		{
			if (iZoom != 0)
			{
				iZoom = 0;
				/*** Back to a centered map. ***/
				iXPosMapMoveOffset = 0;
				iYPosMapMoveOffset = 0;
				PlaySound ("wav/screen2or3.wav");
			}
		}
		if (InAreaMap (252, 809, 252 + 32, 809 + 32) == 1) /*** arrow left ***/
			{ MapAction ("left"); }
		if (InAreaMap (299, 809, 299 + 32, 809 + 32) == 1) /*** arrow up ***/
			{ MapAction ("up"); }
		if (InAreaMap (346, 809, 346 + 32, 809 + 32) == 1) /*** arrow down ***/
			{ MapAction ("down"); }
		if (InAreaMap (393, 809, 393 + 32, 809 + 32) == 1) /*** arrow right ***/
			{ MapAction ("right"); }
		if (InAreaMap (33, 33, 33 + MAP_BIG_AREA_W,
			33 + MAP_BIG_AREA_H) == 1) /*** big area ***/
		{
			if ((iMapHoverRoom != 0) && (iMapMoved == 0))
			{
				iCurRoom = iMapHoverRoom;
				SDL_RaiseWindow (window);
				PlaySound ("wav/scroll.wav");
			}
		}
		if (InAreaMap (636, 807, 636 + 14,
			807 + 14) == 1) /*** show room numbers ***/
		{
			if (iMapShowNumbers == 0)
				{ iMapShowNumbers = 1; }
					else { iMapShowNumbers = 0; }
			PlaySound ("wav/check_box.wav");
		}
	}
	ShowMap();
}
/*****************************************************************************/
void MapKeyDown (SDL_Event event)
/*****************************************************************************/
{
	switch (event.key.keysym.sym)
	{
		case SDLK_0:
		case SDLK_KP_0:
			if (iZoom != 0)
			{
				iZoom = 0;
				/*** Back to a centered map. ***/
				iXPosMapMoveOffset = 0;
				iYPosMapMoveOffset = 0;
				PlaySound ("wav/screen2or3.wav");
			}
			break;
		case SDLK_1:
		case SDLK_KP_1:
			if (ZoomGet() != DEFAULT_ZOOM)
			{
				iZoom = DEFAULT_ZOOM;
				/*** Back to a centered map. ***/
				iXPosMapMoveOffset = 0;
				iYPosMapMoveOffset = 0;
				PlaySound ("wav/screen2or3.wav");
			}
			break;
		case SDLK_KP_PLUS:
		case SDLK_EQUALS:
			MapAction ("in");
			break;
		case SDLK_MINUS:
		case SDLK_KP_MINUS:
			MapAction ("out");
			break;
		case SDLK_LEFT: MapAction ("left"); break;
		case SDLK_UP: MapAction ("up"); break;
		case SDLK_DOWN: MapAction ("down"); break;
		case SDLK_RIGHT: MapAction ("right"); break;
		/*** case SDLK_ESCAPE: ***/
		case SDLK_c:
			MapHide();
			break;
		case SDLK_s:
			if (iMapShowNumbers == 0)
				{ iMapShowNumbers = 1; }
					else { iMapShowNumbers = 0; }
			PlaySound ("wav/check_box.wav");
			break;
		default: break;
	}
	ShowMap();
}
/*****************************************************************************/
void MapMouseMotion (SDL_Event event)
/*****************************************************************************/
{
	int iOldXPos, iOldYPos;

	iOldXPos = iXPosMap;
	iOldYPos = iYPosMap;
	iXPosMap = event.motion.x;
	iYPosMap = event.motion.y;
	if ((iOldXPos == iXPosMap) && (iOldYPos == iYPosMap)) { return; }

	if (InAreaMap (33, 33, 33 + MAP_BIG_AREA_W,
		33 + MAP_BIG_AREA_H) == 0) /*** (not) big area ***/
	{
		/*** Stop map movement. ***/
		iMovingMap = 0;
		SDL_SetCursor (curArrow);
	}

	if (iMovingMap == 1)
	{
		iXPosMapMoveOffset += (iXPosMap - iXPosMapMoveStart);
		iYPosMapMoveOffset += (iYPosMap - iYPosMapMoveStart);
		iXPosMapMoveStart = iXPosMap;
		iYPosMapMoveStart = iYPosMap;

		iMapMoved = 1;
		SDL_SetCursor (curHand);
	}
	ShowMap();
}
/*****************************************************************************/
void MapMouseWheel (SDL_Event event)
/*****************************************************************************/
{
	if (event.wheel.y > 0) /*** scroll wheel up ***/
		{ MapAction ("in"); }
	if (event.wheel.y < 0) /*** scroll wheel down ***/
		{ MapAction ("out"); }
	ShowMap();
}
/*****************************************************************************/
void MapHide (void)
/*****************************************************************************/
{
	/*** iVisible = (SDL_GetWindowFlags (windowmap) & SDL_WINDOW_SHOWN); ***/
	if (iMapOpen != 0)
	{
		SDL_HideWindow (windowmap);

		/*** Back to a centered map. ***/
		iXPosMapMoveOffset = 0;
		iYPosMapMoveOffset = 0;
		iMovingMap = 0;
		SDL_SetCursor (curArrow);

		iMapOpen = 0;
		iMapHoverRoom = 0;
	}
}
/*****************************************************************************/
void ZoomIncrease (void)
/*****************************************************************************/
{
	float fCurrent, fNew;

	fCurrent = ZoomGet();
	fNew = ceil (fCurrent);
	if (fNew == fCurrent) { fNew++; }

	iZoom = fNew;
}
/*****************************************************************************/
void ZoomDecrease (void)
/*****************************************************************************/
{
	float fCurrent, fNew;

	fCurrent = ZoomGet();
	fNew = floor (fCurrent);
	if (fNew == fCurrent) { fNew--; }

	iZoom = fNew;
}
/*****************************************************************************/
void ChangeEvent (int iAmount, int iChangePos)
/*****************************************************************************/
{
	if (((iAmount > 0) && (iChangeEvent != 255)) ||
		((iAmount < 0) && (iChangeEvent != 0)))
	{
		switch (iAmount)
		{
			case -10: iChangeEvent-=10; break;
			case -1: iChangeEvent--; break;
			case 1: iChangeEvent++; break;
			case 10: iChangeEvent+=10; break;
		}
		if (iChangeEvent < 0) { iChangeEvent = 0; }
		if (iChangeEvent > 255) { iChangeEvent = 255; }
		if (iChangePos == 1)
		{
			if ((iThingA[iCurRoom][iSelected] == 0x06) ||
				(iThingA[iCurRoom][iSelected] == 0x0F) ||
				(iThingA[iCurRoom][iSelected] == 0x26) ||
				(iThingA[iCurRoom][iSelected] == 0x2F))
			{
				iModifierA[iCurRoom][iSelected] = iChangeEvent;
				iChanged++;
			}
		}
		PlaySound ("wav/plus_minus.wav");
	}
}
/*****************************************************************************/
void EventRoom (int iRoom)
/*****************************************************************************/
{
	char sBinarySDoors[9 + 2]; /*** 8 chars, plus \0 ***/
	char sBinaryFDoors[9 + 2]; /*** 8 chars, plus \0 ***/
	char sTempBinary[9 + 2]; /*** 8 chars, plus \0 ***/

	/* This part is kind of complicated. Only some bits have
	 * to be changed, so we first convert the event to bits,
	 * change some of them, and then convert all of them back.
	 */
	GetAsEightBits (sFirstDoorEvents[iChangeEvent],
		sBinaryFDoors);
	GetAsEightBits (sSecondDoorEvents[iChangeEvent],
		sBinarySDoors);
	GetAsEightBits (iRoom, sTempBinary);
	sBinarySDoors[0] = sTempBinary[3];
	sBinarySDoors[1] = sTempBinary[4];
	sBinarySDoors[2] = sTempBinary[5];
	sBinarySDoors[3] = '0';
	sBinarySDoors[4] = '0';
	sBinarySDoors[5] = '0';
	sBinarySDoors[6] = '0';
	sBinarySDoors[7] = '0';
	sBinaryFDoors[1] = sTempBinary[6];
	sBinaryFDoors[2] = sTempBinary[7];
	sFirstDoorEvents[iChangeEvent] =
		BitsToInt (sBinaryFDoors);
	sSecondDoorEvents[iChangeEvent] =
		BitsToInt (sBinarySDoors);
	PlaySound ("wav/check_box.wav");
	iChanged++;
}
/*****************************************************************************/
void EventDoor (int iTile)
/*****************************************************************************/
{
	char sBinaryFDoors[9 + 2]; /*** 8 chars, plus \0 ***/
	char sTempBinary[9 + 2]; /*** 8 chars, plus \0 ***/

	/* This part is kind of complicated. Only some bits have
	 * to be changed, so we first convert the event to bits,
	 * change some of them, and then convert them all back.
	 */
	GetAsEightBits (sFirstDoorEvents[iChangeEvent],
		sBinaryFDoors);
	GetAsEightBits (iTile - 1, sTempBinary);
	sBinaryFDoors[3] = sTempBinary[3];
	sBinaryFDoors[4] = sTempBinary[4];
	sBinaryFDoors[5] = sTempBinary[5];
	sBinaryFDoors[6] = sTempBinary[6];
	sBinaryFDoors[7] = sTempBinary[7];
	sFirstDoorEvents[iChangeEvent] =
		BitsToInt (sBinaryFDoors);
	PlaySound ("wav/check_box.wav");
	iChanged++;
}
/*****************************************************************************/
void EventNext (int iNoNext)
/*****************************************************************************/
{
	char sBinaryFDoors[9 + 2]; /*** 8 chars, plus \0 ***/

	/* This part is kind of complicated. Only one bit has
	 * to be changed, so we first convert the event to bits,
	 * change one of them, and then convert them all back.
	 */
	GetAsEightBits (sFirstDoorEvents[iChangeEvent],
		sBinaryFDoors);
	switch (iNoNext)
	{
		case 1: sBinaryFDoors[0] = '1'; break;
		case 0: sBinaryFDoors[0] = '0'; break;
	}
	sFirstDoorEvents[iChangeEvent] =
		BitsToInt (sBinaryFDoors);
	PlaySound ("wav/check_box.wav");
	iChanged++;
}
/*****************************************************************************/
void Help (void)
/*****************************************************************************/
{
	SDL_Event event;
	int iHelp;

	iHelp = 1;

	PlaySound ("wav/popup.wav");
	ShowHelp();
	while (iHelp == 1)
	{
		while (SDL_PollEvent (&event))
		{
			if (MapEvents (event) == 0)
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							iHelp = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_o:
							iHelp = 0; break;
					}
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					if (InArea (79, 344, 79 + 531, 344 + 23) == 1) /*** URL ***/
					{
						SDL_SetCursor (curHand);
					} else {
						SDL_SetCursor (curArrow);
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (588, 410, 588 + 85, 410 + 32) == 1) /*** OK ***/
							{ iHelpOK = 1; }
					}
					ShowHelp();
					break;
				case SDL_MOUSEBUTTONUP:
					iHelpOK = 0;
					if (event.button.button == 1)
					{
						if (InArea (588, 410, 588 + 85, 410 + 32) == 1) /*** OK ***/
							{ iHelp = 0; }
						if (InArea (79, 344, 79 + 531, 344 + 23) == 1) /*** URL ***/
							{ OpenURL ("https://www.norbertdejonge.nl/popyorn/"); }
					}
					ShowHelp();
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED:
							ShowHelp(); break;
						case SDL_WINDOWEVENT_CLOSE:
							Quit(); break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							iActiveWindowID = iWindowID; break;
					}
					break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	SDL_SetCursor (curArrow);
	ShowScreen (iScreen);
}
/*****************************************************************************/
void ShowHelp (void)
/*****************************************************************************/
{
	/*** help ***/
	ShowImage (imghelp, 0, 0, "imghelp", ascreen, iScale, 1);

	/*** OK ***/
	switch (iHelpOK)
	{
		case 0: /*** off ***/
			ShowImage (imgok[1], 588, 410, "imgok[1]", ascreen, iScale, 1);
			break;
		case 1: /*** on ***/
			ShowImage (imgok[2], 588, 410, "imgok[2]", ascreen, iScale, 1);
			break;
	}

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void OpenURL (char *sURL)
/*****************************************************************************/
{
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
ShellExecute (NULL, "open", sURL, NULL, NULL, SW_SHOWNORMAL);
#else
pid_t pid;
pid = fork();
if (pid == 0)
{
	execl ("/usr/bin/xdg-open", "xdg-open", sURL, (char *)NULL);
	exit (EXIT_NORMAL);
}
#endif
}
/*****************************************************************************/
void ApplySkillIfNecessary (int iLoc)
/*****************************************************************************/
{
	if (sGuardLocations[iCurRoom - 1] + 1 == iLoc)
	{
		sGuardSkills[iCurRoom - 1] = iGuardSkill;
		iChanged++;
	}
}
/*****************************************************************************/
void ChangePosCustomAction (char *sAction, int iLocation)
/*****************************************************************************/
{
	if (strcmp (sAction, "save") == 0)
	{
		SetLocation (iCurRoom, iLocation, iChangeGroup, iChangeVariant);
	}
}
/*****************************************************************************/
int ChangePosCustom (int iLocation)
/*****************************************************************************/
{
	int iChanging;
	int iChangingCustom;
	SDL_Event event;
	int iXJoy1, iYJoy1, iXJoy2, iYJoy2;

	iChanging = 1;
	iChangingCustom = 1;

	iChangeGroup = iThingA[iCurRoom][iLocation];
	iChangeVariant = iModifierA[iCurRoom][iLocation];

	ShowChangeCustom();
	while (iChangingCustom == 1)
	{
		while (SDL_PollEvent (&event))
		{
			if (MapEvents (event) == 0)
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							ChangePosCustomAction ("save", iLocation);
							iChanged++;
							iChangingCustom = 0;
							iChanging = 0;
							break;
						case SDL_CONTROLLER_BUTTON_B:
							iChangingCustom = 0; break;
						case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							ChangeCustom (-1, 'g'); break;
						case SDL_CONTROLLER_BUTTON_DPAD_UP:
							ChangeCustom (16, 'g'); break;
						case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							ChangeCustom (-16, 'g'); break;
						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							ChangeCustom (1, 'g'); break;
					}
					ShowChangeCustom();
					break;
				case SDL_CONTROLLERAXISMOTION: /*** triggers and analog sticks ***/
					iXJoy1 = SDL_JoystickGetAxis (joystick, 0);
					iYJoy1 = SDL_JoystickGetAxis (joystick, 1);
					iXJoy2 = SDL_JoystickGetAxis (joystick, 3);
					iYJoy2 = SDL_JoystickGetAxis (joystick, 4);
					if ((iXJoy1 < -30000) || (iXJoy2 < -30000)) /*** left ***/
					{
						if ((SDL_GetTicks() - joyleft) > 300)
						{
							ChangeCustom (-1, 'v');
							joyleft = SDL_GetTicks();
						}
					}
					if ((iXJoy1 > 30000) || (iXJoy2 > 30000)) /*** right ***/
					{
						if ((SDL_GetTicks() - joyright) > 300)
						{
							ChangeCustom (1, 'v');
							joyright = SDL_GetTicks();
						}
					}
					if ((iYJoy1 < -30000) || (iYJoy2 < -30000)) /*** up ***/
					{
						if ((SDL_GetTicks() - joyup) > 300)
						{
							ChangeCustom (16, 'v');
							joyup = SDL_GetTicks();
						}
					}
					if ((iYJoy1 > 30000) || (iYJoy2 > 30000)) /*** down ***/
					{
						if ((SDL_GetTicks() - joydown) > 300)
						{
							ChangeCustom (-16, 'v');
							joydown = SDL_GetTicks();
						}
					}
					ShowChangeCustom();
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_o:
							ChangePosCustomAction ("save", iLocation);
							iChanged++;
							iChangingCustom = 0;
							iChanging = 0;
							break;
						case SDLK_ESCAPE:
						case SDLK_q:
							iChangingCustom = 0; break;
						case SDLK_LEFT:
							if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								ChangeCustom (-16, 'g');
							} else if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT)) {
								ChangeCustom (-1, 'g');
							}
							break;
						case SDLK_RIGHT:
							if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								ChangeCustom (16, 'g');
							} else if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT)) {
								ChangeCustom (1, 'g');
							}
							break;
						case SDLK_UP:
							if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								ChangeCustom (16, 'v');
							} else if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT)) {
								ChangeCustom (1, 'v');
							}
							break;
						case SDLK_DOWN:
							if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								ChangeCustom (-16, 'v');
							} else if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT)) {
								ChangeCustom (-1, 'v');
							}
							break;
					}
					ShowChangeCustom();
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (588, 410, 588 + 85, 410 + 32) == 1) /*** OK ***/
							{ iCustomOK = 1; }
					}
					ShowChangeCustom();
					break;
				case SDL_MOUSEBUTTONUP:
					iCustomOK = 0;
					if (event.button.button == 1)
					{
						/*** group ***/
						if (InArea (45, 64, 45 + 13, 64 + 20) == 1)
							{ ChangeCustom (-16, 'g'); }
						if (InArea (60, 64, 60 + 13, 64 + 20) == 1)
							{ ChangeCustom (-1, 'g'); }
						if (InArea (130, 64, 130 + 13, 64 + 20) == 1)
							{ ChangeCustom (1, 'g'); }
						if (InArea (145, 64, 145 + 13, 64 + 20) == 1)
							{ ChangeCustom (16, 'g'); }

						/*** variant ***/
						if (InArea (203, 64, 203 + 13, 64 + 20) == 1)
							{ ChangeCustom (-16, 'v'); }
						if (InArea (218, 64, 218 + 13, 64 + 20) == 1)
							{ ChangeCustom (-1, 'v'); }
						if (InArea (288, 64, 288 + 13, 64 + 20) == 1)
							{ ChangeCustom (1, 'v'); }
						if (InArea (303, 64, 303 + 13, 64 + 20) == 1)
							{ ChangeCustom (16, 'v'); }

						if (InArea (588, 410, 588 + 85, 410 + 32) == 1) /*** OK ***/
						{
							ChangePosCustomAction ("save", iLocation);
							iChanged++;
							iChangingCustom = 0;
							iChanging = 0;
						}
					}
					ShowChangeCustom();
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED:
							ShowChangeCustom(); break;
						case SDL_WINDOWEVENT_CLOSE:
							Quit(); break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							iActiveWindowID = iWindowID; break;
					}
					break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	if (iChanging == 1) { PlaySound ("wav/ok_close.wav"); }

	return (iChanging);
}
/*****************************************************************************/
void ChangeCustom (int iAmount, char cGroupOrVariant)
/*****************************************************************************/
{
	switch (cGroupOrVariant)
	{
		case 'g': /*** group ***/
			if (((iAmount > 0) && (iChangeGroup != 255)) ||
				((iAmount < 0) && (iChangeGroup != 0)))
			{
				iChangeGroup += iAmount;
				if (iChangeGroup < 0) { iChangeGroup = 0; }
				if (iChangeGroup > 255) { iChangeGroup = 255; }
				PlaySound ("wav/plus_minus.wav");
			}
			break;
		case 'v': /*** variant ***/
			if (((iAmount > 0) && (iChangeVariant != 255)) ||
				((iAmount < 0) && (iChangeVariant != 0)))
			{
				iChangeVariant += iAmount;
				if (iChangeVariant < 0) { iChangeVariant = 0; }
				if (iChangeVariant > 255) { iChangeVariant = 255; }
				PlaySound ("wav/plus_minus.wav");
			}
			break;
	}
}
/*****************************************************************************/
void ShowChangeCustom (void)
/*****************************************************************************/
{
	ShowImage (imgcustom, 0, 0, "imgcustom", ascreen, iScale, 1);

	switch (iCustomOK)
	{
		case 0: /*** off ***/
			ShowImage (imgok[1], 588, 410, "imgok[1]",
				ascreen, iScale, 1); break;
		case 1: /*** on ***/
			ShowImage (imgok[2], 588, 410, "imgok[2]",
				ascreen, iScale, 1); break;
	}

	CenterNumber (ascreen, iChangeGroup, 73, 64, color_bl, 1);
	CenterNumber (ascreen, iChangeVariant, 231, 64, color_bl, 1);

	ShowTile (iChangeGroup, iChangeVariant, 105);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void CopyPaste (int iRoom, int iAction)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iLoopTile;

	if (iAction == 1) /*** copy ***/
	{
		for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{
			iThingACopyPaste[iLoopTile] = iThingA[iRoom][iLoopTile];
			iModifierACopyPaste[iLoopTile] = iModifierA[iRoom][iLoopTile];
		}
		iGuardACopyPaste[0] = sGuardLocations[iRoom - 1];
		iGuardACopyPaste[1] = sGuardDirections[iRoom - 1];
		iGuardACopyPaste[2] = sGuardSkills[iRoom - 1];
		iGuardACopyPaste[3] = sGuardColors[iRoom - 1];
		iCopied = 1;
	} else { /*** paste ***/
		for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{
			if (iCopied == 1)
			{
				iThingA[iRoom][iLoopTile] = iThingACopyPaste[iLoopTile];
				iModifierA[iRoom][iLoopTile] = iModifierACopyPaste[iLoopTile];
			} else {
				iThingA[iRoom][iLoopTile] = 0x00;
				iModifierA[iRoom][iLoopTile] = 0x00;
			}
		}
		if (iCopied == 1)
		{
			sGuardLocations[iRoom - 1] = iGuardACopyPaste[0];
			sGuardDirections[iRoom - 1] = iGuardACopyPaste[1];
			sGuardSkills[iRoom - 1] = iGuardACopyPaste[2];
			sGuardColors[iRoom - 1] = iGuardACopyPaste[3];
		} else {
			sGuardLocations[iRoom - 1] = 30;
		}
	}
}
/*****************************************************************************/
void Zoom (int iToggleFull)
/*****************************************************************************/
{
	if (iToggleFull == 1)
	{
		if (iFullscreen == 0)
			{ iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP; }
				else { iFullscreen = 0; }
	} else {
		if (iFullscreen == SDL_WINDOW_FULLSCREEN_DESKTOP)
		{
			iFullscreen = 0;
			iScale = 1;
		} else if (iScale == 1) {
			iScale = 2;
		} else if (iScale == 2) {
			iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
		} else {
			printf ("[ WARN ] Unknown window state!\n");
		}
	}

	SDL_SetWindowFullscreen (window, iFullscreen);
	SDL_SetWindowSize (window, (WINDOW_WIDTH) * iScale,
		(WINDOW_HEIGHT) * iScale);
	SDL_RenderSetLogicalSize (ascreen, (WINDOW_WIDTH) * iScale,
		(WINDOW_HEIGHT) * iScale);
	SDL_SetWindowPosition (window, SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED);
	TTF_CloseFont (font11);
	TTF_CloseFont (font15);
	TTF_CloseFont (font20);
	LoadFonts();
}
/*****************************************************************************/
void EXE (void)
/*****************************************************************************/
{
	int iEXE;
	SDL_Event event;

	iEXE = 1;
	iStatusBarFrame = 1;
	snprintf (sStatus, MAX_STATUS, "%s", "");

	EXELoad();

	PlaySound ("wav/popup.wav");
	ShowEXE();
	while (iEXE == 1)
	{
		if (iNoAnim == 0)
		{
			/*** Using the global REFRESH. No need for newticks/oldticks. ***/
			iStatusBarFrame++;
			if (iStatusBarFrame == 19) { iStatusBarFrame = 1; }
			ShowEXE();
		}

		while (SDL_PollEvent (&event))
		{
			if (MapEvents (event) == 0)
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							EXESave(); iEXE = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							iEXE = 0; break;
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_s:
							EXESave(); iEXE = 0; break;
					}
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					UpdateStatusBar();
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (588, 410, 588 + 85, 410 + 32) == 1) /*** Save ***/
							{ iEXESave = 1; }
					}
					ShowEXE();
					break;
				case SDL_MOUSEBUTTONUP:
					iEXESave = 0;
					if (event.button.button == 1)
					{
						if (InArea (588, 410, 588 + 85, 410 + 32) == 1) /*** Save ***/
							{ EXESave(); iEXE = 0; }

						/*** Starting minutes left. ***/
						PlusMinus (&iEXEMinutesLeft, 217, 33, 0, 999, -10, 0);
						PlusMinus (&iEXEMinutesLeft, 232, 33, 0, 999, -1, 0);
						PlusMinus (&iEXEMinutesLeft, 302, 33, 0, 999, +1, 0);
						PlusMinus (&iEXEMinutesLeft, 317, 33, 0, 999, +10, 0);

						/*** Starting hit points. ***/
						PlusMinus (&iEXEHitPoints, 217, 57, 0, 999, -10, 0);
						PlusMinus (&iEXEHitPoints, 232, 57, 0, 999, -1, 0);
						PlusMinus (&iEXEHitPoints, 302, 57, 0, 999, +1, 0);
						PlusMinus (&iEXEHitPoints, 317, 57, 0, 999, +10, 0);

						/*** Skip potions level. ***/
						if ((InArea (259, 84, 259 + 14, 84 + 14) == 1) &&
							(iEXESkipPotions == 1))
							{ iEXESkipPotions = 0; PlaySound ("wav/check_box.wav"); }
						if ((InArea (274, 84, 274 + 14, 84 + 14) == 1) &&
							(iEXESkipPotions == 0))
							{ iEXESkipPotions = 1; PlaySound ("wav/check_box.wav"); }

						/*** Win room. ***/
						PlusMinus (&iEXEWinRoom, 217, 105, 1, 24, -10, 0);
						PlusMinus (&iEXEWinRoom, 232, 105, 1, 24, -1, 0);
						PlusMinus (&iEXEWinRoom, 302, 105, 1, 24, +1, 0);
						PlusMinus (&iEXEWinRoom, 317, 105, 1, 24, +10, 0);
					}
					ShowEXE();
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_EXPOSED:
							ShowEXE(); break;
						case SDL_WINDOWEVENT_CLOSE:
							Quit(); break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							iActiveWindowID = iWindowID; break;
					}
					break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	ShowScreen (iScreen);
}
/*****************************************************************************/
void ShowEXE (void)
/*****************************************************************************/
{
	SDL_Color clr;
	char arText[1 + 2][MAX_TEXT + 2];

	/*** exe ***/
	ShowImage (imgexe, 0, 0, "imgexe", ascreen, iScale, 1);

	/*** status bar ***/
	if (strcmp (sStatus, "") != 0)
	{
		/*** bulb ***/
		ShowImage (imgstatusbarsprite, 23, 415, "imgstatusbarsprite",
			ascreen, iScale, 1);
		/*** text ***/
		snprintf (arText[0], MAX_TEXT, "%s", sStatus);
		DisplayText (50, 419, 11, arText, 1, font11, color_f4, color_bl, 0);
	}

	/*** save ***/
	switch (iEXESave)
	{
		case 0: /*** off ***/
			ShowImage (imgsave[1], 588, 410, "imgsave[1]",
				ascreen, iScale, 1); break;
		case 1: /*** on ***/
			ShowImage (imgsave[2], 588, 410, "imgsave[2]",
				ascreen, iScale, 1); break;
	}

	/*** Starting minutes left. ***/
	if (iEXEMinutesLeft == 60) { clr = color_bl; } else { clr = color_blue; }
	CenterNumber (ascreen, iEXEMinutesLeft, 245, 33, clr, 0);

	/*** Starting hit points. ***/
	if (iEXEHitPoints == 3) { clr = color_bl; } else { clr = color_blue; }
	CenterNumber (ascreen, iEXEHitPoints, 245, 57, clr, 0);

	/*** Skip potions level. ***/
	if (iEXESkipPotions == 0)
	{
		ShowImage (imgsell, 259, 84, "imgsell", ascreen, iScale, 1);
	} else {
		ShowImage (imgsell, 274, 84, "imgsell", ascreen, iScale, 1);
		ShowImage (imgsrs, 274, 84, "imgsrs", ascreen, iScale, 1);
	}

	/*** Win room. ***/
	if (iEXEWinRoom == 5) { clr = color_bl; } else { clr = color_blue; }
	CenterNumber (ascreen, iEXEWinRoom, 245, 105, clr, 0);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void EXELoad (void)
/*****************************************************************************/
{
	int iFdEXE;
	unsigned char sData[MAX_DATA + 2];

	iFdEXE = open (sPathFile, O_RDONLY|O_BINARY);
	if (iFdEXE == -1)
	{
		printf ("[FAILED] Error opening %s: %s!\n", sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	/*** Starting minutes left. ***/
	lseek (iFdEXE, OFFSET_STARTMIN, SEEK_SET);
	ReadFromFile (iFdEXE, "", 2, sData);
	iEXEMinutesLeft = BytesAsLU (sData, 2);

	/*** Starting hit points. ***/
	lseek (iFdEXE, OFFSET_STARTHP, SEEK_SET);
	ReadFromFile (iFdEXE, "", 2, sData);
	iEXEHitPoints = BytesAsLU (sData, 2);

	/*** Skip potions level. ***/
	lseek (iFdEXE, OFFSET_SKIPPOT, SEEK_SET);
	ReadFromFile (iFdEXE, "", 1, sData);
	switch (BytesAsLU (sData, 1))
	{
		case 0x66: iEXESkipPotions = 0; break;
		case 0x60: iEXESkipPotions = 1; break;
		default:
			printf ("[ WARN ] Unknown potions level status: %lu!\n",
				BytesAsLU (sData, 2));
			iEXESkipPotions = 0; /*** Fallback. ***/
			break;
	}

	/*** Win room. ***/
	lseek (iFdEXE, OFFSET_WINROOM, SEEK_SET);
	ReadFromFile (iFdEXE, "", 2, sData);
	iEXEWinRoom = BytesAsLU (sData, 2);

	close (iFdEXE);
}
/*****************************************************************************/
void EXESave (void)
/*****************************************************************************/
{
	int iFdEXE;

	iFdEXE = open (sPathFile, O_RDWR|O_BINARY);
	if (iFdEXE == -1)
	{
		printf ("[FAILED] Error opening %s: %s!\n", sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	/*** Starting minutes left. ***/
	lseek (iFdEXE, OFFSET_STARTMIN, SEEK_SET);
	WriteWord (iFdEXE, iEXEMinutesLeft);

	/*** Starting hit points. ***/
	lseek (iFdEXE, OFFSET_STARTHP, SEEK_SET);
	WriteWord (iFdEXE, iEXEHitPoints);

	/*** Skip potions level. ***/
	lseek (iFdEXE, OFFSET_SKIPPOT, SEEK_SET);
	switch (iEXESkipPotions)
	{
		case 0: WriteByte (iFdEXE, 0x66); break;
		case 1: WriteByte (iFdEXE, 0x60); break;
	}

	/*** Win room. ***/
	lseek (iFdEXE, OFFSET_WINROOM, SEEK_SET);
	WriteWord (iFdEXE, iEXEWinRoom);

	close (iFdEXE);

	PlaySound ("wav/save.wav");
}
/*****************************************************************************/
void UpdateStatusBar (void)
/*****************************************************************************/
{
	snprintf (sStatusOld, MAX_STATUS, "%s", sStatus);
	snprintf (sStatus, MAX_STATUS, "%s", "");
	if (InArea (217, 83, 217 + 113, 83 + 16) == 1) /*** Skip potions level. ***/
	{
		snprintf (sStatus, MAX_STATUS, "%s", "Crack the game?");
	}
	if (strcmp (sStatus, sStatusOld) != 0) { ShowEXE(); }
}
/*****************************************************************************/
int PlusMinus (int *iWhat, int iX, int iY,
	int iMin, int iMax, int iChange, int iAddChanged)
/*****************************************************************************/
{
	if ((InArea (iX, iY, iX + 13, iY + 20) == 1) &&
		(((iChange < 0) && (*iWhat > iMin)) ||
		((iChange > 0) && (*iWhat < iMax))))
	{
		*iWhat = *iWhat + iChange;
		if ((iChange < 0) && (*iWhat < iMin)) { *iWhat = iMin; }
		if ((iChange > 0) && (*iWhat > iMax)) { *iWhat = iMax; }
		if (iAddChanged == 1) { iChanged++; }
		PlaySound ("wav/plus_minus.wav");
		return (1);
	} else { return (0); }
}
/*****************************************************************************/
unsigned long BytesAsLU (unsigned char *sData, int iBytes)
/*****************************************************************************/
{
	/*** This function expects big-endian data. ***/

	unsigned long luReturn;
	char sString[MAX_DATA + 2];
	char sTemp[MAX_DATA + 2];
	int iTemp;

	snprintf (sString, MAX_DATA, "%s", "");
	for (iTemp = iBytes - 1; iTemp >= 0; iTemp--)
	{
		snprintf (sTemp, MAX_DATA, "%02x%s", sData[iTemp], sString);
		snprintf (sString, MAX_DATA, "%s", sTemp);
	}
	luReturn = strtoul (sString, NULL, 16);

	return (luReturn);
}
/*****************************************************************************/
void WriteByte (int iFd, int iValue)
/*****************************************************************************/
{
	char sToWrite[MAX_TOWRITE + 2];
	int iWritten;

	snprintf (sToWrite, MAX_TOWRITE, "%c", iValue);
	iWritten = write (iFd, sToWrite, 1);
	if (iWritten == -1)
	{
		printf ("[ WARN ] Could not write: %s!", strerror (errno));
	}
}
/*****************************************************************************/
void WriteWord (int iFd, int iValue)
/*****************************************************************************/
{
	/*** This function writes big-endian data. ***/

	char sToWrite[MAX_TOWRITE + 2];
	int iWritten;

	snprintf (sToWrite, MAX_TOWRITE, "%c%c", (iValue >> 8) & 0xFF,
		(iValue >> 0) & 0xFF);
	iWritten = write (iFd, sToWrite, 2);
	if (iWritten == -1)
	{
		printf ("[ WARN ] Could not write: %s!", strerror (errno));
	}
}
/*****************************************************************************/
void CreateBAK (void)
/*****************************************************************************/
{
	FILE *fBIN;
	FILE *fBAK;
	int iData;

	fBIN = fopen (sPathFile, "rb");
	if (fBIN == NULL)
		{ printf ("[ WARN ] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno)); }

	fBAK = fopen (BACKUP, "wb");
	if (fBAK == NULL)
		{ printf ("[ WARN ] Could not open \"%s\": %s!\n",
			BACKUP, strerror (errno)); }

	while (1)
	{
		iData = fgetc (fBIN);
		if (iData == EOF) { break; }
			else { putc (iData, fBAK); }
	}

	fclose (fBIN);
	fclose (fBAK);
}
/*****************************************************************************/
int OnLevelBar (void)
/*****************************************************************************/
{
	if (InArea (28, 3, 28 + 575, 3 + 19) == 1)
		{ return (1); } else { return (0); }
}
/*****************************************************************************/
void RunLevel (int iLevel)
/*****************************************************************************/
{
	SDL_Thread *princethread;

	ModifyForPlaytest (iLevel);
	princethread = SDL_CreateThread (StartGame, "StartGame", NULL);
	if (princethread == NULL)
		{ printf ("[ WARN ] Could not create thread!\n"); }
}
/*****************************************************************************/
int StartGame (void *unused)
/*****************************************************************************/
{
	if (unused != NULL) { } /*** To prevent warnings. ***/

	PlaySound ("wav/playtest.wav");
	if (system (HERE BATCH_FILE) == -1)
		{ printf ("[ WARN ] Could not execute batch file!\n"); }
	if (iModified == 1) { ModifyBack(); }

	return (EXIT_NORMAL);
}
/*****************************************************************************/
void ModifyForPlaytest (int iLevel)
/*****************************************************************************/
{
	int iFd;
	unsigned char sData[MAX_DATA + 2];

	iFd = open (sPathFile, O_RDWR|O_BINARY);
	if (iFd == -1)
	{
		printf ("[ WARN ] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno));
	}

	/*** Store current value. ***/
	lseek (iFd, OFFSET_STARTLVL, SEEK_SET);
	ReadFromFile (iFd, "", 2, sData);
	iEXEStartLevel = BytesAsLU (sData, 2);

	/*** Change the New Game level to the active in-editor level. ***/
	lseek (iFd, OFFSET_STARTLVL, SEEK_SET);
	WriteWord (iFd, iLevel);

	close (iFd);

	iModified = 1;
}
/*****************************************************************************/
void ModifyBack (void)
/*****************************************************************************/
{
	int iFd;

	iFd = open (sPathFile, O_RDWR|O_BINARY);
	if (iFd == -1)
	{
		printf ("[ WARN ] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno));
	}

	/*** Change the New Game level back. ***/
	lseek (iFd, OFFSET_STARTLVL, SEEK_SET);
	WriteWord (iFd, iEXEStartLevel);

	close (iFd);

	iModified = 0;
}
/*****************************************************************************/
void Sprinkle (void)
/*****************************************************************************/
{
	int iCmpThing, iCmpMod;
	int iRandom;

	/*** Used for looping. ***/
	int iLoopRoom;
	int iLoopTile;

	for (iLoopRoom = 1; iLoopRoom <= ROOMS; iLoopRoom++)
	{
		for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{
			iCmpThing = iThingA[iLoopRoom][iLoopTile];
			if (iCmpThing > 32) { iCmpThing-=32; }
			iCmpMod = iModifierA[iLoopRoom][iLoopTile];

			if (cCurType == 'd')
			{
				/*** empty? add wall pattern ***/
				if (((iCmpThing == 0x00) && (iCmpMod == 0x00)) ||
					((iCmpThing == 0x00) && (iCmpMod == 0xFF)))
				{
					/*** 1-4 ***/
					iRandom = 1 + (int) (4.0 * rand() / (RAND_MAX + 1.0));
					switch (iRandom)
					{
						case 1:
							/*** iThingA unchanged ***/
							iModifierA[iLoopRoom][iLoopTile] = 0x01;
							break;
						case 2:
							/*** iThingA unchanged ***/
							iModifierA[iLoopRoom][iLoopTile] = 0x02;
							break;
					}
				}
				/* empty floor? add wall pattern (2x), rubble, torch,
				 * skeleton, torch + rubble
				 */
				if (((iCmpThing == 0x01) && (iCmpMod == 0x00)) ||
					((iCmpThing == 0x01) && (iCmpMod == 0xFF)))
				{
					/*** 1-12 ***/
					iRandom = 1 + (int) (12.0 * rand() / (RAND_MAX + 1.0));
					switch (iRandom)
					{
						case 1:
							iThingA[iLoopRoom][iLoopTile] = 0x01;
							iModifierA[iLoopRoom][iLoopTile] = 0x01;
							break;
						case 2:
							iThingA[iLoopRoom][iLoopTile] = 0x01;
							iModifierA[iLoopRoom][iLoopTile] = 0x02;
							break;
						case 3:
							iThingA[iLoopRoom][iLoopTile] = 0x0E;
							iModifierA[iLoopRoom][iLoopTile] = 0x00;
							break;
						case 4:
							iThingA[iLoopRoom][iLoopTile] = 0x13;
							iModifierA[iLoopRoom][iLoopTile] = 0x00;
							break;
						case 5:
							iThingA[iLoopRoom][iLoopTile] = 0x15;
							iModifierA[iLoopRoom][iLoopTile] = 0x00;
							break;
						case 6:
							iThingA[iLoopRoom][iLoopTile] = 0x1E;
							iModifierA[iLoopRoom][iLoopTile] = 0x00;
							break;
					}
				}
			}
			if (cCurType == 'p')
			{
				/*** simple wall pattern? add variant pattern ***/
				if ((iCmpThing == 0x00) && (iCmpMod == 0x01))
				{
					/*** 1-2 ***/
					iRandom = 1 + (int) (2.0 * rand() / (RAND_MAX + 1.0));
					if (iRandom == 1)
					{
						/*** iThingA unchanged ***/
						iModifierA[iLoopRoom][iLoopTile] = 0x02;
					}
				}
				/* empty floor? add variant pattern, rubble, torch,
				 * torch + rubble
				 */
				if (((iCmpThing == 0x01) && (iCmpMod == 0x00)) ||
					((iCmpThing == 0x01) && (iCmpMod == 0xFF)))
				{
					/*** 1-8 ***/
					iRandom = 1 + (int) (8.0 * rand() / (RAND_MAX + 1.0));
					switch (iRandom)
					{
						case 1:
							iThingA[iLoopRoom][iLoopTile] = 0x01;
							iModifierA[iLoopRoom][iLoopTile] = 0x02;
							break;
						case 2:
							iThingA[iLoopRoom][iLoopTile] = 0x0E;
							iModifierA[iLoopRoom][iLoopTile] = 0x00;
							break;
						case 3:
							iThingA[iLoopRoom][iLoopTile] = 0x13;
							iModifierA[iLoopRoom][iLoopTile] = 0x00;
							break;
						case 4:
							iThingA[iLoopRoom][iLoopTile] = 0x1E;
							iModifierA[iLoopRoom][iLoopTile] = 0x00;
							break;
					}
				}
			}
		}
	}
}
/*****************************************************************************/
void FlipRoom (int iRoom, int iAxis)
/*****************************************************************************/
{
	int iTileUse;
	int iThingTempA[30 + 2];
	int iModifierTempA[30 + 2];
	int iKidPC, iGuardPC;

	/*** Used for looping. ***/
	int iLoopTile;

	/*** Storing tiles. ***/
	for (iLoopTile = 0; iLoopTile <= TILES; iLoopTile++)
	{
		iThingTempA[iLoopTile] = iThingA[iRoom][iLoopTile];
		iModifierTempA[iLoopTile] = iModifierA[iRoom][iLoopTile];
	}

	if (iAxis == 1) /*** horizontal ***/
	{
		/*** tiles ***/
		for (iLoopTile = 0; iLoopTile <= TILES; iLoopTile++)
		{
			iTileUse = 1; /*** To prevent warnings. ***/
			if ((iLoopTile >= 1) && (iLoopTile <= 10))
				{ iTileUse = 11 - iLoopTile; }
			if ((iLoopTile >= 11) && (iLoopTile <= 20))
				{ iTileUse = 31 - iLoopTile; }
			if ((iLoopTile >= 21) && (iLoopTile <= 30))
				{ iTileUse = 51 - iLoopTile; }
			iThingA[iRoom][iLoopTile] = iThingTempA[iTileUse];
			iModifierA[iRoom][iLoopTile] = iModifierTempA[iTileUse];
		}

		/*** prince ***/
		if ((int)luKidRoom == iRoom)
		{
			/*** looking direction ***/
			if (luKidDir == 0) { luKidDir = 255; } else { luKidDir = 0; }
			/*** horizontal position ***/
			iKidPC = (int)luKidPos;
			if ((iKidPC >= 1) && (iKidPC <= 10)) { luKidPos = 11 - iKidPC; }
			if ((iKidPC >= 11) && (iKidPC <= 20)) { luKidPos = 31 - iKidPC; }
			if ((iKidPC >= 21) && (iKidPC <= 30)) { luKidPos = 51 - iKidPC; }
		}

		/*** guard ***/
		if (sGuardLocations[iRoom - 1] < 30)
		{
			/*** looking direction ***/
			if (sGuardDirections[iRoom - 1] == 0)
				{ sGuardDirections[iRoom - 1] = 255; }
					else { sGuardDirections[iRoom - 1] = 0; }
			/*** horizontal position ***/
			iGuardPC = sGuardLocations[iRoom - 1] + 1;
			if ((iGuardPC >= 1) && (iGuardPC <= 10))
				{ sGuardLocations[iRoom - 1] = 11 - (iGuardPC + 1); }
			if ((iGuardPC >= 11) && (iGuardPC <= 20))
				{ sGuardLocations[iRoom - 1] = 31 - (iGuardPC + 1); }
			if ((iGuardPC >= 21) && (iGuardPC <= 30))
				{ sGuardLocations[iRoom - 1] = 51 - (iGuardPC + 1); }
		}
	} else { /*** vertical ***/
		/*** tiles ***/
		for (iLoopTile = 1; iLoopTile <= TILES; iLoopTile++)
		{
			iTileUse = 1; /*** To prevent warnings. ***/
			if ((iLoopTile >= 1) && (iLoopTile <= 10))
				{ iTileUse = iLoopTile + 20; }
			if ((iLoopTile >= 11) && (iLoopTile <= 20))
				{ iTileUse = iLoopTile; }
			if ((iLoopTile >= 21) && (iLoopTile <= 30))
				{ iTileUse = iLoopTile - 20; }
			iThingA[iRoom][iLoopTile] = iThingTempA[iTileUse];
			iModifierA[iRoom][iLoopTile] = iModifierTempA[iTileUse];
		}

		/*** prince ***/
		if ((int)luKidRoom == iRoom)
		{
			/*** vertical position ***/
			iKidPC = (int)luKidPos;
			if ((iKidPC >= 1) && (iKidPC <= 10)) { luKidPos = iKidPC + 20; }
			if ((iKidPC >= 21) && (iKidPC <= 30)) { luKidPos = iKidPC - 20; }
		}

		/*** guard ***/
		if (sGuardLocations[iRoom - 1] < 30)
		{
			/*** vertical position ***/
			iGuardPC = sGuardLocations[iRoom - 1] + 1;
			if ((iGuardPC >= 1) && (iGuardPC <= 10))
				{ sGuardLocations[iRoom - 1] = (iGuardPC - 1) + 20; }
			if ((iGuardPC >= 21) && (iGuardPC <= 30))
				{ sGuardLocations[iRoom - 1] = (iGuardPC - 1) - 20; }
		}
	}
}
/*****************************************************************************/
void GetModifierAsName (int iFore, int iMod, char *sName)
/*****************************************************************************/
{
	int iForeR;

	iForeR = iFore;
	if (iForeR >= 192) { iForeR-=192; }
	if (iForeR >= 128) { iForeR-=128; }
	if (iForeR >= 64) { iForeR-=64; }
	if (iForeR >= 32) { iForeR-=32; }

	snprintf (sName, 9, "%s", "???????");
	switch (iForeR)
	{
		case 0:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "+Not-Bl"); break;
				case 1: snprintf (sName, 9, "%s", "+Spt1-N"); break;
				case 2: snprintf (sName, 9, "%s", "+Spt2-D"); break;
				case 3: snprintf (sName, 9, "%s", "Window "); break;
				case 6: snprintf (sName, 9, "%s", "POTIONS"); break;
				case 78: snprintf (sName, 9, "%s", "POTIONS"); break;
				case 144: snprintf (sName, 9, "%s", "POTIONS"); break;
				case 255: snprintf (sName, 9, "%s", "+Spt3-B"); break;
			}
			break;
		case 1:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "+Not-Bl"); break;
				case 1: snprintf (sName, 9, "%s", "+Spt1-N"); break;
				case 2: snprintf (sName, 9, "%s", "+Spt2-D"); break;
				case 3: snprintf (sName, 9, "%s", "Window "); break;
				case 6: snprintf (sName, 9, "%s", "POTIONS"); break;
				case 78: snprintf (sName, 9, "%s", "POTIONS"); break;
				case 144: snprintf (sName, 9, "%s", "POTIONS"); break;
				case 255: snprintf (sName, 9, "%s", "+Spt3-B"); break;
			}
			break;
		case 2:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "Normal "); break;
				case 1: snprintf (sName, 9, "%s", "BarelyO"); break;
				case 2: snprintf (sName, 9, "%s", "HalfOut"); break;
				case 3: snprintf (sName, 9, "%s", "FulyOut"); break;
				case 4: snprintf (sName, 9, "%s", "FulyOut"); break;
				case 5: snprintf (sName, 9, "%s", "Out    "); break;
				case 6: snprintf (sName, 9, "%s", "Out    "); break;
				case 7: snprintf (sName, 9, "%s", "HalfOut"); break;
				case 8: snprintf (sName, 9, "%s", "BarelyO"); break;
				case 9: snprintf (sName, 9, "%s", "Disabld"); break;
			}
			break;
		case 3: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 4:
			switch (iMod)
			{
				/*** says the documentation ***/
				case 0: snprintf (sName, 9, "%s", "Closed "); break;
				case 1: snprintf (sName, 9, "%s", "Open   "); break;
				/*** says the game ***/
				case 2: snprintf (sName, 9, "%s", "Closed "); break;
			}
			break;
		case 5: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 6: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 7:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "-WithLt"); break;
				case 1: snprintf (sName, 9, "%s", "-AlterD"); break;
				case 2: snprintf (sName, 9, "%s", "-Normal"); break;
				case 3: snprintf (sName, 9, "%s", "-Black "); break;
				case 4: snprintf (sName, 9, "%s", "-Black "); break;
				case 5: snprintf (sName, 9, "%s", "-AltDes"); break;
				case 6: snprintf (sName, 9, "%s", "-WthBtm"); break;
				case 7: snprintf (sName, 9, "%s", "-WthWin"); break;
			}
			break;
		case 8: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 9: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 10:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "Empty  "); break;
				case 1: snprintf (sName, 9, "%s", "HealthP"); break;
				case 2: snprintf (sName, 9, "%s", "Life   "); break;
				case 3: snprintf (sName, 9, "%s", "FeatFal"); break;
				case 4: snprintf (sName, 9, "%s", "Invert "); break;
				case 5: snprintf (sName, 9, "%s", "Poison "); break;
				case 6: snprintf (sName, 9, "%s", "Open   "); break;
				case 32: snprintf (sName, 9, "%s", "PRINCES"); break;
				case 64: snprintf (sName, 9, "%s", "PRINCES"); break;
				case 160: snprintf (sName, 9, "%s", "PRINCES"); break;
			}
			break;
		case 11: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 12:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "-WithLt"); break;
				case 1: snprintf (sName, 9, "%s", "-AlterD"); break;
				case 2: snprintf (sName, 9, "%s", "-Normal"); break;
				case 3: snprintf (sName, 9, "%s", "-Black "); break;
			}
			break;
		case 13: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 14: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 15: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 16: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 17:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "Always "); break;
				case 255: snprintf (sName, 9, "%s", "POTIONS"); break;
			}
			break;
		case 18:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "Normal "); break;
				case 1: snprintf (sName, 9, "%s", "HalfOpn"); break;
				case 2: snprintf (sName, 9, "%s", "Closed "); break;
				case 3: snprintf (sName, 9, "%s", "PartOpn"); break;
				case 4: snprintf (sName, 9, "%s", "ExtrOpn"); break;
				case 5: snprintf (sName, 9, "%s", "StckOpn"); break;
			}
			break;
		case 19: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 20:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "+Norm-B"); break;
				case 1: snprintf (sName, 9, "%s", "+Norm-N"); break;
			}
			break;
		case 21: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 22: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 23: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 24: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 25: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 26: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 27: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 28: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 29: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 30: if (iMod == 0) { snprintf (sName, 9, "%s", "Always "); } break;
		case 31:
			switch (iMod)
			{
				case 0: snprintf (sName, 9, "%s", "Always "); break;
				case 253: snprintf (sName, 9, "%s", "POTIONS"); break;
				case 255: snprintf (sName, 9, "%s", "POTIONS"); break;
			}
			break;
	}
}
/*****************************************************************************/
