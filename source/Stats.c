#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "StrFunc.h"
#include "AkelEdit.h"
#include "AkelDLL.h"


//Include string functions
#define xstrlenW
#define xstrcpynW
#define xatoiW
#define xitoaW
#define xuitoaW
#define dec2hexW
#define xprintfW
#include "StrFunc.h"

//Include AEC functions
#define AEC_FUNCTIONS
#include "AkelEdit.h"

//Defines
#define STRID_INFO   1
#define STRID_PLUGIN 2

//#define BUFFER_SIZE       1024

//Functions prototypes
const char* GetLangStringA(LANGID wLangID, int nStringID);
const wchar_t* GetLangStringW(LANGID wLangID, int nStringID);
void InitCommon(PLUGINDATA *pd);

//Global variables
wchar_t wszBuffer[BUFFER_SIZE];
wchar_t wszPluginName[MAX_PATH];
wchar_t wszPluginTitle[MAX_PATH];
HWND hMainWnd;
HWND hWndEdit;
BOOL bOldWindows;
BOOL bAkelEdit;
LANGID wLangModule;
BOOL bInitCommon=FALSE;
DWORD dwStatusPosType=0;


//Identification
void __declspec(dllexport) DllAkelPadID(PLUGINVERSION *pv)
{
  pv->dwAkelDllVersion=AKELDLL;
  pv->dwExeMinVersion3x=MAKE_IDENTIFIER(-1, -1, -1, -1);
  pv->dwExeMinVersion4x=MAKE_IDENTIFIER(4, 9, 7, 0);
  pv->pPluginName="Stats";
}

//Plugin extern function
void __declspec(dllexport) Main(PLUGINDATA *pd)
{
  //Function doesn't support autoload
  pd->dwSupport|=PDS_NOAUTOLOAD;
  if (pd->dwSupport & PDS_GETSUPPORT)
    return;

  InitCommon(pd);

  //Calculate
  {
    AECHARRANGE crInit;
    AECHARRANGE crCount;
    AECHARINDEX ciWordStart;
    AECHARINDEX ciWordEnd;
    AECHARINDEX ciCount;
    AEINDEXSUBTRACT is;
    INT_PTR nCharCount=0;
    INT_PTR nCharLatinSP=0;
    INT_PTR nCharLatinLetters=0;
    INT_PTR nCharLatinDigits=0;
//  INT_PTR nCharLatinOther=0;
    INT_PTR nCharFWLatinLetters=0;
    INT_PTR nCharFWLatinDigits=0;
    INT_PTR nCharLatinRN=0;
    INT_PTR nCharLatinNonRN=0;
    INT_PTR nCharNonLatin=0;
    INT_PTR nCharWLatin1Letters=0;
    INT_PTR nCharWLatin2Letters=0;
    INT_PTR nCharSurrogate=0;
    INT_PTR nCharPl1=0;
    INT_PTR nCharPl2=0;
    INT_PTR nCharPl3=0;
    INT_PTR nCharPl14=0;
    INT_PTR nCharPl15_16=0;
    INT_PTR nCharHan=0;
    INT_PTR nCharHanRad=0;
    INT_PTR nCharHanExtA=0;
    INT_PTR nCharHanExtB_F=0;
    INT_PTR nCharHanExtG_H=0;
    INT_PTR nCharHanCmp1=0;
    INT_PTR nCharHanCmp2=0;
    INT_PTR nCharPUA=0;
    INT_PTR nCharNonLatinSP=0;
    int nWordCount=0;
    int nLineCount=0;
    BOOL bFirst=TRUE;

    if (hWndEdit)
    {
      SendMessage(hWndEdit, AEM_GETINDEX, AEGI_FIRSTSELCHAR, (LPARAM)&crInit.ciMin);
      SendMessage(hWndEdit, AEM_GETINDEX, AEGI_LASTSELCHAR, (LPARAM)&crInit.ciMax);

       /* whole file */
      if (!AEC_IndexCompare(&crInit.ciMin, &crInit.ciMax))
      {
        SendMessage(hWndEdit, AEM_GETINDEX, AEGI_FIRSTCHAR, (LPARAM)&crInit.ciMin);
        SendMessage(hWndEdit, AEM_GETINDEX, AEGI_LASTCHAR, (LPARAM)&crInit.ciMax);

        //Line count
        nLineCount=(crInit.ciMax.nLine - crInit.ciMin.nLine) + 1;
        if (!(dwStatusPosType & SPT_LINEWRAP))
          nLineCount=(int)SendMessage(hWndEdit, AEM_GETUNWRAPLINE, nLineCount - 1, 0) + 1;

        //Char count
        is.ciChar1=&crInit.ciMax;
        is.ciChar2=&crInit.ciMin;
        is.bColumnSel=FALSE;
        is.nNewLine=AELB_ASOUTPUT;
        nCharCount=SendMessage(hWndEdit, AEM_INDEXSUBTRACT, 0, (LPARAM)&is);

        //Word count
        ciCount=crInit.ciMin;

        for (;;)
        {
          if (!SendMessage(hWndEdit, AEM_GETNEXTBREAK, AEWB_RIGHTWORDEND, (LPARAM)&ciCount))
            break;

          ++nWordCount;
        }

        //Symbols detalization
        for (ciCount=crInit.ciMin; AEC_IndexCompare(&ciCount, &crInit.ciMax) < 0; AEC_NextChar(&ciCount))
        {
          if (ciCount.nCharInLine < ciCount.lpLine->nLineLen)
          {
            if (AEC_IndexLen(&ciCount) == 1)
            {
              if (ciCount.lpLine->wpLine[ciCount.nCharInLine] < 0x80)
              {			// ==== \u00-\u7F - Basic Latin
                ++nCharLatinNonRN;
                     if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] == L' '  ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == L'\t' ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == L'\v' ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == L'\f')  { ++nCharLatinSP;      }
                else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] >= L'A' &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] <= L'Z') ||
                         (ciCount.lpLine->wpLine[ciCount.nCharInLine] >= L'a' &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] <= L'z'))  { ++nCharLatinLetters; }
                else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] >= L'0' &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] <= L'9' )  { ++nCharLatinDigits;  }
                //else ++nCharLatinOther;
              }
              else {	// ==== \u80-\uFFFF - NonLatin
                ++nCharNonLatin;
                // --------------------------------------------------
                if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x024f )
                {		//Latin: 0080-00FF:Latin-1+; 0100-017F,0180-024F:Ext-A,B;
                       if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x0100   ) { ++nCharWLatin2Letters; }
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x00C0 &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] != 0x00D7 &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] != 0x00F7)  ) { ++nCharWLatin1Letters; }//
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x00A0   ) { ++nCharNonLatinSP;}// No-Break Space [NBSP]
                }
                else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x2E80 )
                {
                       if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x9FFF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x4E00)  ) { ++nCharHan;       }// CJK Unified Ideographs
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x3000   ) { ++nCharNonLatinSP;}// Ideographic space
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x4DBF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x3400)  ) { ++nCharHanExtA;   }// CJK Unified Ideographs Ext-A
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x2FDF   ) { ++nCharHanRad;    }// CJK Radicals Supplement,Kangxi Radicals
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xF8FF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xE000)  ) { ++nCharPUA;       }// PUA
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFAFF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xF900)  ) { ++nCharHanCmp1;   }// CJK Compatibility Ideographs
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFF3A &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xFF21) ||
                           (ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFF5A &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xFF41) )  { ++nCharFWLatinLetters; }// Fullwidth latin letters
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFF19 &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xFF10  )  { ++nCharFWLatinDigits;  }// Fullwidth digits
                }
                else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x200A &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x2000) ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x2028  || // Line separator
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x2029  || // Paragraph separator
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x202F  || // Narrow No-Break Space[NNBSP]
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x205F  || // Medium Mathematical Space[MMSP]
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x0085  || // cc Next Line[NEL]
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x1680     // Ogham Space Mark
                        ) { ++nCharNonLatinSP; }// Whitespace (White_Space=true)
                        // Ignorable: 0x200B[ZWSP], 0x200C[ZWNJ], 0x200D[ZWJ], U+2060[WJ], 0xFEFF[BOM, ZWNBSP]
                // TODO
                // J Kana  : 3040..30FF:Hiragana,Katakana; 31F0-31FF:Katakana Phonetic Ext;   1AFF0-1B16F:Kana Ext-A..B,Supplement,Small Kana Ext; ...
                // K Hangul: 1100-11FF:Jamo; 3130-318F:Compat Jamo; A960-A97F:Jamo Ext-A; AC00-D7AF,D7B0-D7FF:Syllables,Jamo Ext-B; ...
                // --------------------------------------------------
              }
            }
            else {		// === Surrogate
              // ----------------------------------------------->
              ++nCharSurrogate;
              // ========= Plane 1: U+10000-1FFFF [SMP]
              if        ( AEC_CharAtIndex(&ciCount) <= 0x1FFFF ) { ++nCharPl1;  }
              // ========= Plane 2: U+20000-2FFFF [SIP]
              else if   ( AEC_CharAtIndex(&ciCount) <= 0x2FFFF ) { ++nCharPl2; 
                     if ( AEC_CharAtIndex(&ciCount) <= 0x2EBEF ) {   ++nCharHanExtB_F; } // CJK Unified Ideographs Ext-B..F
                else if ( AEC_CharAtIndex(&ciCount) >= 0x2F800 &&
                          AEC_CharAtIndex(&ciCount) <= 0x2FA1F ) {   ++nCharHanCmp2;   } // CJK Compatibility Ideographs Supplement
              }
              // ========= Plane 3: U+30000-3FFFF [TIP]
              else if   ( AEC_CharAtIndex(&ciCount) <= 0x3FFFF ) { ++nCharPl3; 
                     if ( AEC_CharAtIndex(&ciCount) <= 0x323AF ) {   ++nCharHanExtG_H; } // CJK Unified Ideographs Ext-G..H
              } 
              // ========= Plane 4-13: unassigned
              // ========= Plane 14: U+E0000-EFFFF [SSP]
              else if   ( AEC_CharAtIndex(&ciCount) >= 0xE0000 &&
                          AEC_CharAtIndex(&ciCount) <= 0xEFFFF)  { ++nCharPl14;  }
              // ========= Planes 15-16: U+F0000-FFFFF, 100000-10FFFF [SPUA-A/B]
              else if   ( AEC_CharAtIndex(&ciCount) >= 0xF0000 &&
                          AEC_CharAtIndex(&ciCount) <= 0x10FFFF) { ++nCharPl15_16; }
              // -----------------------------------------------<
            }
          }
          else {		// == CR+LF
            if      (ciCount.lpLine->nLineBreak == AELB_R || 
                     ciCount.lpLine->nLineBreak == AELB_N)  ++nCharLatinRN;
            else if (ciCount.lpLine->nLineBreak == AELB_RN)   nCharLatinRN+=2;
            else if (ciCount.lpLine->nLineBreak == AELB_RRN)  nCharLatinRN+=3;
          }
        }
      }
       /* selection */
      else
      {
        //Line count
        if (!(dwStatusPosType & SPT_LINEWRAP))
          nLineCount=(int)SendMessage(hWndEdit, AEM_GETUNWRAPLINE, crInit.ciMax.nLine, 0) - (int)SendMessage(hWndEdit, AEM_GETUNWRAPLINE, crInit.ciMin.nLine, 0) + 1;
        else
          nLineCount=(crInit.ciMax.nLine - crInit.ciMin.nLine) + 1;

        //Char count
        is.ciChar1=&crInit.ciMax;
        is.ciChar2=&crInit.ciMin;
        is.bColumnSel=(BOOL)SendMessage(hWndEdit, AEM_GETCOLUMNSEL, 0, 0);
        is.nNewLine=AELB_ASOUTPUT;
        nCharCount=SendMessage(hWndEdit, AEM_INDEXSUBTRACT, 0, (LPARAM)&is);

        //Word count
        if (is.bColumnSel)
        {
          crCount.ciMin=crInit.ciMin;
          crCount.ciMax=crInit.ciMax;

          while (AEC_IndexCompare(&crCount.ciMin, &crCount.ciMax) < 0)
          {
            ciWordEnd=crCount.ciMin;

            for (;;)
            {
              if (!SendMessage(hWndEdit, AEM_GETNEXTBREAK, AEWB_RIGHTWORDEND, (LPARAM)&ciWordEnd))
                goto Symbols;

              if (ciWordEnd.nLine == crCount.ciMin.nLine && ciWordEnd.nCharInLine <= crCount.ciMin.lpLine->nSelEnd)
              {
                if (bFirst)
                {
                  bFirst=FALSE;
                  ciWordStart=ciWordEnd;

                  if (SendMessage(hWndEdit, AEM_GETPREVBREAK, AEWB_LEFTWORDSTART, (LPARAM)&ciWordStart))
                    if (AEC_IndexCompare(&crCount.ciMin, &ciWordStart) <= 0)
                      ++nWordCount;
                }
                else ++nWordCount;

                if (ciWordEnd.nCharInLine == crCount.ciMin.lpLine->nSelEnd)
                  break;
              }
              else break;
            }

            //Next line
            bFirst=TRUE;
            if (AEC_NextLine(&crCount.ciMin))
              crCount.ciMin.nCharInLine=crCount.ciMin.lpLine->nSelStart;
          }
        }
        else
        {
          crCount.ciMin=crInit.ciMin;
          crCount.ciMax=crInit.ciMax;

          for (;;)
          {
            ciWordEnd=crCount.ciMin;

            if (!SendMessage(hWndEdit, AEM_GETNEXTBREAK, AEWB_RIGHTWORDEND, (LPARAM)&ciWordEnd))
              goto Symbols;

            if (AEC_IndexCompare(&ciWordEnd, &crCount.ciMax) <= 0)
            {
              if (bFirst)
              {
                bFirst=FALSE;
                ciWordStart=ciWordEnd;

                if (SendMessage(hWndEdit, AEM_GETPREVBREAK, AEWB_LEFTWORDSTART, (LPARAM)&ciWordStart))
                  if (AEC_IndexCompare(&crCount.ciMin, &ciWordStart) <= 0)
                    ++nWordCount;
              }
              else ++nWordCount;

              if (AEC_IndexCompare(&ciWordEnd, &crCount.ciMax) == 0)
                break;
            }
            else break;

            //Next word
            crCount.ciMin=ciWordEnd;
          }
        }

        //Symbols detalization
        Symbols:

        for (ciCount=crInit.ciMin; AEC_IndexCompare(&ciCount, &crInit.ciMax) < 0; AEC_NextChar(&ciCount))
        {
          if (ciCount.nCharInLine < ciCount.lpLine->nSelStart)
              ciCount.nCharInLine = ciCount.lpLine->nSelStart;

          if (ciCount.nCharInLine < ciCount.lpLine->nLineLen &&
              ciCount.nCharInLine < ciCount.lpLine->nSelEnd)
          {
            if (AEC_IndexLen(&ciCount) == 1)
            {
              if (ciCount.lpLine->wpLine[ciCount.nCharInLine] < 0x80)
              {			// ==== \u00-\u7F - Basic Latin
                ++nCharLatinNonRN;
                     if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] == L' '  ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == L'\t' ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == L'\v' ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == L'\f')  { ++nCharLatinSP;      }
                else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] >= L'A' &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] <= L'Z') ||
                         (ciCount.lpLine->wpLine[ciCount.nCharInLine] >= L'a' &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] <= L'z'))  { ++nCharLatinLetters; }
                else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] >= L'0' &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] <= L'9' )  { ++nCharLatinDigits;  }
                //else ++nCharLatinOther;
              }
              else {	// ==== \u80-\uFFFF - NonLatin
                ++nCharNonLatin;
                // --------------------------------------------------
                if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x024f )
                {		//Latin: 80-FF:Latin-1+; 0100-017F,0180-024F:Ext-A,B;
                       if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x0100   ) { ++nCharWLatin2Letters; }
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x00C0 &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] != 0x00D7 &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] != 0x00F7)  ) { ++nCharWLatin1Letters; }//
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x00A0   ) { ++nCharNonLatinSP;}// No-Break Space [NBSP]
                }
                else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x2E80 )
                {
                       if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x9FFF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x4E00)  ) { ++nCharHan;       }// CJK Unified Ideographs
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x3000   ) { ++nCharNonLatinSP;}// Ideographic space
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x4DBF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x3400)  ) { ++nCharHanExtA;   }// CJK Unified Ideographs Ext-A
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x2FDF   ) { ++nCharHanRad;    }// CJK Radicals Supplement,Kangxi Radicals
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xF8FF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xE000)  ) { ++nCharPUA;       }// PUA
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFAFF &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xF900)  ) { ++nCharHanCmp1;   }// CJK Compatibility Ideographs
                  else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFF3A &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xFF21) ||
                           (ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFF5A &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xFF41) )  { ++nCharFWLatinLetters; }// Fullwidth latin letters
                  else if ( ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0xFF19 &&
                            ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0xFF10  )  { ++nCharFWLatinDigits;  }// Fullwidth digits
                }
                else if ((ciCount.lpLine->wpLine[ciCount.nCharInLine] <= 0x200A &&
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] >= 0x2000) ||
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x2028  || // Line separator
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x2029  || // Paragraph separator
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x202F  || // Narrow No-Break Space[NNBSP]
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x205F  || // Medium Mathematical Space[MMSP]
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x0085  || // cc Next Line[NEL]
                          ciCount.lpLine->wpLine[ciCount.nCharInLine] == 0x1680     // Ogham Space Mark
                        ) { ++nCharNonLatinSP; }// Whitespace (White_Space=true)
                        // Ignorable: 0x200B[ZWSP], 0x200C[ZWNJ], 0x200D[ZWJ], U+2060[WJ], 0xFEFF[BOM, ZWNBSP]
                // TODO
                // J Kana  : 3040..30FF:Hiragana,Katakana; 31F0-31FF:Katakana Phonetic Ext;   1AFF0-1B16F:Kana Ext-A..B,Supplement,Small Kana Ext; ...
                // K Hangul: 1100-11FF:Jamo; 3130-318F:Compat Jamo; A960-A97F:Jamo Ext-A; AC00-D7AF,D7B0-D7FF:Syllables,Jamo Ext-B; ...
                // --------------------------------------------------
              }
            }
            else {		// === Surrogate
              // ----------------------------------------------->
              ++nCharSurrogate;
              // ========= Plane 1: U+10000-1FFFF [SMP]
              if        ( AEC_CharAtIndex(&ciCount) <= 0x1FFFF ) { ++nCharPl1;  }
              // ========= Plane 2: U+20000-2FFFF [SIP]
              else if   ( AEC_CharAtIndex(&ciCount) <= 0x2FFFF ) { ++nCharPl2; 
                     if ( AEC_CharAtIndex(&ciCount) <= 0x2EBEF ) {   ++nCharHanExtB_F; } // CJK Unified Ideographs Ext-B..F
                else if ( AEC_CharAtIndex(&ciCount) >= 0x2F800 &&
                          AEC_CharAtIndex(&ciCount) <= 0x2FA1F ) {   ++nCharHanCmp2;   } // CJK Compatibility Ideographs Supplement
              }
              // ========= Plane 3: U+30000-3FFFF [TIP]
              else if   ( AEC_CharAtIndex(&ciCount) <= 0x3FFFF ) { ++nCharPl3; 
                     if ( AEC_CharAtIndex(&ciCount) <= 0x323AF ) {   ++nCharHanExtG_H; } // CJK Unified Ideographs Ext-G..H
              } 
              // ========= Plane 4-13: unassigned
              // ========= Plane 14: U+E0000-EFFFF [SSP]
              else if   ( AEC_CharAtIndex(&ciCount) >= 0xE0000 &&
                          AEC_CharAtIndex(&ciCount) <= 0xEFFFF)  { ++nCharPl14;  }
              // ========= Planes 15-16: U+F0000-FFFFF, 100000-10FFFF [SPUA-A/B]
              else if   ( AEC_CharAtIndex(&ciCount) >= 0xF0000 &&
                          AEC_CharAtIndex(&ciCount) <= 0x10FFFF) { ++nCharPl15_16; }
              // -----------------------------------------------<
            }
          }
          else {		// == CR+LF
            if      (ciCount.lpLine->nLineBreak == AELB_R || 
                     ciCount.lpLine->nLineBreak == AELB_N)  ++nCharLatinRN;
            else if (ciCount.lpLine->nLineBreak == AELB_RN)   nCharLatinRN+=2;
            else if (ciCount.lpLine->nLineBreak == AELB_RRN)  nCharLatinRN+=3;
            //AEC_NextLine(&ciCount);
          }
        }
      }
    }

    //Show result
		xprintfW(wszBuffer, GetLangStringW(wLangModule, STRID_INFO), 
					nCharCount, 
						nCharLatinNonRN + nCharNonLatin + nCharSurrogate + nCharLatinRN,
						nCharLatinNonRN + nCharNonLatin + nCharSurrogate - nCharLatinSP - nCharNonLatinSP,
						nLineCount, 
					nCharLatinNonRN + nCharLatinRN + nCharNonLatin, 
					nCharNonLatin, nCharPUA,
					nCharLatinRN, nCharLatinSP, nCharNonLatinSP, 
					nCharLatinLetters, nCharWLatin1Letters, nCharWLatin2Letters, nCharFWLatinLetters,
					nCharLatinDigits,  nCharFWLatinDigits,
					nCharHan,
					nCharHanExtA,   nCharHanCmp1,
					nCharHanExtB_F, nCharHanCmp2, nCharPl2,
					nCharHanExtG_H,               nCharPl3, 
					nCharPl1,  nCharPl14, nCharPl15_16,
					nWordCount);
    MessageBoxW(hMainWnd, wszBuffer, wszPluginTitle, MB_OK|MB_ICONINFORMATION);
  }
}

const char* GetLangStringA(LANGID wLangID, int nStringID)
{
  static char szStringBuf[MAX_PATH];

  WideCharToMultiByte(CP_ACP, 0, GetLangStringW(wLangID, nStringID), -1, szStringBuf, MAX_PATH, NULL, NULL);
  return szStringBuf;
}

const wchar_t* GetLangStringW(LANGID wLangID, int nStringID)
{
/*  if (wLangID == LANG_RUSSIAN)
  {
    if (nStringID == STRID_INFO)
      return L"\x0421\x0438\x043C\x0432\x043E\x043B\x044B=%Id\n    \x041B\x0430\x0442\x0438\x043D\x0441\x043A\x0438\x0435=%Id\n        \x0411\x0443\x043A\x0432\x044B=%Id\n        \x0426\x0438\x0444\x0440\x044B=%Id\n        \x041F\x0440\x043E\x0431\x0435\x043B\x044B=%Id\n        \x0414\x0440\x0443\x0433\x0438\x0435=%Id\n    \x041D\x0435\x043B\x0430\x0442\x0438\x043D\x0441\x043A\x0438\x0435=%Id\n    \x0421\x0443\x0440\x0440\x043E\x0433\x0430\x0442\x044B=%Id\n\x0421\x043B\x043E\x0432\x0430=%d\n\x0421\x0442\x0440\x043E\x043A\x0438=%d\n";
    if (nStringID == STRID_PLUGIN)
      return L"%s \x043F\x043B\x0430\x0433\x0438\x043D";
  }
  else
    {*/
  if (nStringID == STRID_INFO)
    return L"\
%-9Id\tChars \n\
%-9Id\tCharacters (multibyte safe) \n\
%-9Id\tCharacters, non-white \n\
%-9Id\tLines \n\
------------------------\n\
%-9Id\tCharacters from Plane 0 [BMP] \n\
%-9Id\tnon-basicLatin\t%-8Id\tPUA\n\
------------------------\n\
-Whitespaces- \n\
  CR/LF    \t: %-8Id\tBasicLatin\t: %-8Id\tNonLatin \t: %-8Id\n\n\
-Latin Letters- \n\
  Basic    \t: %-8Id\tLatin1\t: %-8Id\tExt-A..B\t: %-8Id\n\
  Fullwidth\t: %-8Id\t\n\n\
-Digits- \n\
  Basic    \t: %-8Id\t\n\
  Fullwidth\t: %-8Id\t\n\n\
-CJK Ideographs- \n\
  Unified \t: %-8Id\t \n\
  Ext-A   \t: %-8Id\tCompat.\t: %-8Id\t\n\
  Ext-B..F\t: %-8Id\tCompat+\t: %-8Id\t[SIP]\t: %-8Id\n\
  Ext-G..H\t: %-8Id\t       \t       \t[TIP]\t: %-8Id\n\
------------------------\n\
[SMP]\t%-8Id\t\n\
[SSP]\t%-8Id\t[SPUA]\t%-8Id\t\n\
------------------------\n\
Words=%d\n";

  if (nStringID == STRID_PLUGIN)
    return L"%s plugin";
//  }
  return L"";
}

void InitCommon(PLUGINDATA *pd)
{
  bInitCommon=TRUE;
  hMainWnd=pd->hMainWnd;
  hWndEdit=pd->hWndEdit;
  bOldWindows=pd->bOldWindows;
  bAkelEdit=pd->bAkelEdit;
  wLangModule=PRIMARYLANGID(pd->wLangModule);

  //Plugin name
  {
    int i;

    for (i=0; pd->wszFunction[i] != L':'; ++i)
      wszPluginName[i]=pd->wszFunction[i];
    wszPluginName[i]=L'\0';
  }
  xprintfW(wszPluginTitle, GetLangStringW(wLangModule, STRID_PLUGIN), wszPluginName);

  dwStatusPosType=(DWORD)SendMessage(hMainWnd, AKD_GETMAININFO, MI_STATUSPOSTYPE, 0);
}

//Entry point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  if (fdwReason == DLL_PROCESS_ATTACH)
  {
  }
  else if (fdwReason == DLL_THREAD_ATTACH)
  {
  }
  else if (fdwReason == DLL_THREAD_DETACH)
  {
  }
  else if (fdwReason == DLL_PROCESS_DETACH)
  {
  }
  return TRUE;
}
