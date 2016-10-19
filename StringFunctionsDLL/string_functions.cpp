//******************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//******************************************************************************
//
// Purpose: String functions library
//
//******************************************************************************

// Needs to be defined here, before #include "window_functions.h"
// to generate proper export- and inport-definitions
#define STRING_FUNCTIONS_EXPORTS

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#include <assert.h>
#include <math.h>
#include <windows.h>
#include "assert.h"
#include "string_functions.h"
#include "..\OpenHoldem\MagicNumbers.h"
#include "Psapi.h"

const char kUnprintableBeepChar = 0x07;
const char kCharToBeRemoved = kUnprintableBeepChar;

CString Bool2CString(bool b) {
  return (b ? "true" : "false");
}

CString CStringRemoveLeft(CString string, int number_of_characters_to_remove) {
  int length = string.GetLength();
  int number_of_character_to_keep = length - number_of_characters_to_remove;
  return string.Right(number_of_character_to_keep);
}

CString CStringRemoveRight(CString string, int number_of_characters_to_remove) {
  int length = string.GetLength();
  int number_of_character_to_keep = length - number_of_characters_to_remove;
  return string.Left(number_of_character_to_keep);
}

int  DigitCharacterToNumber(char digit) {
  if ((digit >= '0') && (digit <= '9')) {
    return (digit - '0');
  }
  // Actually we want to return kUndefined (-1)
  // but the result gets used as an index without any additional checks,
  // so we return something "fail-safe" for the error-case.
  return kUndefinedZero;
}

CString IntToBinaryString(int number, int min_digits /* = 1 */) {
  CString result;
  assert(min_digits >= 1);
  assert(min_digits <= 32);
  if (number < 0) {
    result = "-";
    number = 0 - number;
  }
  result += "0b";
  CString digits;
  do {
    if ((number % 2) == 0) {
      digits = "0" + digits;
    }
    else {
      digits = "1" + digits;
    }
    number /= 2;
    --min_digits;
  } while ((number > 0) || (min_digits > 0));
  result += digits;
  return result;
};

CString Number2CString(double number, int default_precision /* = 2 */) {
  assert(default_precision >= 0);
  CString result;
  // Avoiding the IsInteger function used by OH,
  // as we don't want to include too much in this DLL.
  // Replacement taken from 
  // http://stackoverflow.com/questions/1521607/check-double-variable-if-it-contains-an-integer-and-not-floating-point
  if (trunc(number) == number) {
    result.Format("%d", int(number));
  } else {
    CString format_string;
    format_string.Format("%%1.%if", default_precision);
    result.Format(format_string, number);
  }
  return result;
}

void RemoveLeftWhiteSpace(CString *s) {
  s->TrimLeft();
}

void RemoveMultipleWhiteSpaces(CString *s) {
  // Remove all sequences or more white spaces,
  // keep only the first white space.
  int length = s->GetLength();
  for (int i = 0; i < (length - 1); ++i) {
    switch (s->GetAt(i)) {
    case ' ':
    case '\t':
    case '\n':
    case kCharToBeRemoved:
      char next_char = s->GetAt(i + 1);
      switch (next_char) {
      case ' ':
      case '\t':
      case '\n':
        s->SetAt(i + 1, kCharToBeRemoved);
        break;
      }
      break;
    }
  }
  // Finally remove all superfluous characters
  s->Remove(kCharToBeRemoved);
}

void RemoveRightWhiteSpace(CString *s) {
  s->TrimRight();
}

void RemoveSpacesInsideNumbers(CString *s) {
  int length = s->GetLength();
  // Looking for patters like "7 500"
  // last_index is the last position where the first digit might occur.
  int last_index = length - 4;
  for (int i = 0; i < last_index; ++i) {
    if (isdigit(s->GetAt(i))
      && isspace(s->GetAt(i + 1))
      && isdigit(s->GetAt(i + 2))
      && isdigit(s->GetAt(i + 3))
      && isdigit(s->GetAt(i + 4))
      // Accessing +5 is save;
      // If things go bad it is \0
      && !isdigit(s->GetAt(i + 5))) {
      s->SetAt(i + 1, kCharToBeRemoved);
    }
  }
  // Finally remove all superfluous characters
  s->Remove(kCharToBeRemoved);
}

void ReplaceCommasInNumbersByDots(CString *s) {
  int length = s->GetLength();
  int last_index = length - 1;
  for (int i = 0; i < last_index; ++i) {
    if ((s->GetAt(i) == ',')
      && isdigit(s->GetAt(i - 1))
      && isdigit(s->GetAt(i + 1))) {
      s->SetAt(i, '.');
    }
  }
}

void ReplaceOutlandischCurrencyByDollarsandCents(CString *s) {
  int length = s->GetLength();
  for (int i = 0; i < length; ++i) {
    char current_char = s->GetAt(i);
    switch (current_char) {
    case '�':
    case '�': s->SetAt(i, '$');
      break;
    case '�': s->SetAt(i, 'c');
      break;
    }
  }
}

void RemoveSpacesInFrontOfCentMultipliers(CString *s) {
  int length = s->GetLength();
  int last_index = length - 2;
  for (int i = 0; i < last_index; ++i) {
    if (isdigit(s->GetAt(i))
      && (isspace(s->GetAt(i + 1)))
      && (s->GetAt(i + 2) == 'c')) {
      s->SetAt(i + 1, kCharToBeRemoved);
    }
  }
  // Finally remove all superfluous characters
  s->Remove(kCharToBeRemoved);
}

void RemoveExtraDotsInNumbers(CString *s) {
  bool inside_number = false;
  bool dot_inside_number_seen = false;
  int length = s->GetLength();
  int last_index = length - 1;
  // Searching backwards for dots in numbers
  for (int i = last_index; i >= 0; --i) {
    char current_char = s->GetAt(i);
    if (isdigit(current_char)) {
      inside_number = true;
      continue;
    }
    else if (current_char == '.') {
      if (dot_inside_number_seen) {
        s->SetAt(i, kCharToBeRemoved);
      }
      dot_inside_number_seen = true;
      continue;
    }
    else {
      inside_number = false;
      dot_inside_number_seen = false;
    }
  }
  // Finally remove all superfluous characters
  s->Remove(kCharToBeRemoved);
}

// !!! call this!
void StringFunctionsSelfTest() {
#ifdef _DEBUG
  CString crappy_title = "  Robostars  Buyin �5,666.777,8     Ante 250 000      Rake 25 �     ";
  ReplaceOutlandischCurrencyByDollarsandCents(&crappy_title);
  crappy_title = "  Robostars  Buyin $5,666.777,8     Ante 250 000      Rake 25 c     ";
  RemoveLeftWhiteSpace(&crappy_title);
  assert(crappy_title == "Robostars  Buyin $5,666.777,8     Ante 250 000      Rake 25 c     ");
  RemoveRightWhiteSpace(&crappy_title);
  assert(crappy_title == "Robostars  Buyin $5,666.777,8     Ante 250 000      Rake 25 c");
  RemoveMultipleWhiteSpaces(&crappy_title);
  assert(crappy_title == "Robostars Buyin $5,666.777,8 Ante 250 000 Rake 25 c");
  RemoveSpacesInsideNumbers(&crappy_title);
  assert(crappy_title == "Robostars Buyin $5,666.777,8 Ante 250000 Rake 25 c");
  RemoveSpacesInFrontOfCentMultipliers(&crappy_title);
  assert(crappy_title == "Robostars Buyin $5,666.777,8 Ante 250000 Rake 25c");
  ReplaceCommasInNumbersByDots(&crappy_title);
  assert(crappy_title == "Robostars Buyin $5.666.777.8 Ante 250000 Rake 25c");
  RemoveExtraDotsInNumbers(&crappy_title);
  assert(crappy_title == "Robostars Buyin $5666777.8 Ante 250000 Rake 25c");
#endif
}

char RightCharacter(CString string, int index_from_right /* = 0 */) {
  assert(index_from_right >= 0);
  int index_from_left = string.GetLength() - index_from_right - 1;
  return string[index_from_left];
}

int RightDigitCharacterToNumber(CString string, int index_from_right /* = 0 */) {
  assert(index_from_right >= 0);
  return DigitCharacterToNumber(RightCharacter(string, index_from_right));
}

bool StringAIsPrefixOfStringB(const char *string_a, const char *string_b) {
  if (strlen(string_a) > strlen(string_b)) {
    return false;
  }
  // Result of memcmp == 0 means: identical
  // See http://www.cplusplus.com/reference/clibrary/cstring/memcmp/
  return (memcmp(string_a, string_b, strlen(string_a)) == 0);
}

bool StringIsExactMatch(const char *string_a, const char *string_b) {
  return ((strlen(string_a) == strlen(string_b))
    && StringAIsPrefixOfStringB(string_a, string_b));
}