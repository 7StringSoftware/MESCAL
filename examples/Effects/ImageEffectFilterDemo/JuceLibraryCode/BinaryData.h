/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Buttons_png;
    const int            Buttons_pngSize = 36934;

    extern const char*   EffectParameters_json;
    const int            EffectParameters_jsonSize = 7673;

    extern const char*   Piet_Mondriaan_19391942__Composition_10_jpg;
    const int            Piet_Mondriaan_19391942__Composition_10_jpgSize = 345743;

    extern const char*   VanGoghstarry_night_jpg;
    const int            VanGoghstarry_night_jpgSize = 355658;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 4;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
