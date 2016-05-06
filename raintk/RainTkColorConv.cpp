/*
   Copyright (C) 2016 Preet Desai (preet.desai@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <raintk/RainTkColorConv.hpp>
#include <raintk/RainTkGlobal.hpp>

#include <map>

namespace raintk
{
    // =========================================================== //

    ColorInvalid::ColorInvalid() :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,"",false)
    {}

    // =========================================================== //

    namespace Color
    {
        namespace
        {
            // 2 character byte lookup table
            std::map<std::string,u8> const g_lkup_byte_str =
            {
                {"00",0},
                {"01",1},
                {"02",2},
                {"03",3},
                {"04",4},
                {"05",5},
                {"06",6},
                {"07",7},
                {"08",8},
                {"09",9},
                {"0A",10},
                {"0a",10},
                {"0B",11},
                {"0b",11},
                {"0C",12},
                {"0c",12},
                {"0D",13},
                {"0d",13},
                {"0E",14},
                {"0e",14},
                {"0F",15},
                {"0f",15},
                {"10",16},
                {"11",17},
                {"12",18},
                {"13",19},
                {"14",20},
                {"15",21},
                {"16",22},
                {"17",23},
                {"18",24},
                {"19",25},
                {"1A",26},
                {"1a",26},
                {"1B",27},
                {"1b",27},
                {"1C",28},
                {"1c",28},
                {"1D",29},
                {"1d",29},
                {"1E",30},
                {"1e",30},
                {"1F",31},
                {"1f",31},
                {"20",32},
                {"21",33},
                {"22",34},
                {"23",35},
                {"24",36},
                {"25",37},
                {"26",38},
                {"27",39},
                {"28",40},
                {"29",41},
                {"2A",42},
                {"2a",42},
                {"2B",43},
                {"2b",43},
                {"2C",44},
                {"2c",44},
                {"2D",45},
                {"2d",45},
                {"2E",46},
                {"2e",46},
                {"2F",47},
                {"2f",47},
                {"30",48},
                {"31",49},
                {"32",50},
                {"33",51},
                {"34",52},
                {"35",53},
                {"36",54},
                {"37",55},
                {"38",56},
                {"39",57},
                {"3A",58},
                {"3a",58},
                {"3B",59},
                {"3b",59},
                {"3C",60},
                {"3c",60},
                {"3D",61},
                {"3d",61},
                {"3E",62},
                {"3e",62},
                {"3F",63},
                {"3f",63},
                {"40",64},
                {"41",65},
                {"42",66},
                {"43",67},
                {"44",68},
                {"45",69},
                {"46",70},
                {"47",71},
                {"48",72},
                {"49",73},
                {"4A",74},
                {"4a",74},
                {"4B",75},
                {"4b",75},
                {"4C",76},
                {"4c",76},
                {"4D",77},
                {"4d",77},
                {"4E",78},
                {"4e",78},
                {"4F",79},
                {"4f",79},
                {"50",80},
                {"51",81},
                {"52",82},
                {"53",83},
                {"54",84},
                {"55",85},
                {"56",86},
                {"57",87},
                {"58",88},
                {"59",89},
                {"5A",90},
                {"5a",90},
                {"5B",91},
                {"5b",91},
                {"5C",92},
                {"5c",92},
                {"5D",93},
                {"5d",93},
                {"5E",94},
                {"5e",94},
                {"5F",95},
                {"5f",95},
                {"60",96},
                {"61",97},
                {"62",98},
                {"63",99},
                {"64",100},
                {"65",101},
                {"66",102},
                {"67",103},
                {"68",104},
                {"69",105},
                {"6A",106},
                {"6a",106},
                {"6B",107},
                {"6b",107},
                {"6C",108},
                {"6c",108},
                {"6D",109},
                {"6d",109},
                {"6E",110},
                {"6e",110},
                {"6F",111},
                {"6f",111},
                {"70",112},
                {"71",113},
                {"72",114},
                {"73",115},
                {"74",116},
                {"75",117},
                {"76",118},
                {"77",119},
                {"78",120},
                {"79",121},
                {"7A",122},
                {"7a",122},
                {"7B",123},
                {"7b",123},
                {"7C",124},
                {"7c",124},
                {"7D",125},
                {"7d",125},
                {"7E",126},
                {"7e",126},
                {"7F",127},
                {"7f",127},
                {"80",128},
                {"81",129},
                {"82",130},
                {"83",131},
                {"84",132},
                {"85",133},
                {"86",134},
                {"87",135},
                {"88",136},
                {"89",137},
                {"8A",138},
                {"8a",138},
                {"8B",139},
                {"8b",139},
                {"8C",140},
                {"8c",140},
                {"8D",141},
                {"8d",141},
                {"8E",142},
                {"8e",142},
                {"8F",143},
                {"8f",143},
                {"90",144},
                {"91",145},
                {"92",146},
                {"93",147},
                {"94",148},
                {"95",149},
                {"96",150},
                {"97",151},
                {"98",152},
                {"99",153},
                {"9A",154},
                {"9a",154},
                {"9B",155},
                {"9b",155},
                {"9C",156},
                {"9c",156},
                {"9D",157},
                {"9d",157},
                {"9E",158},
                {"9e",158},
                {"9F",159},
                {"9f",159},
                {"A0",160},
                {"a0",160},
                {"A1",161},
                {"a1",161},
                {"A2",162},
                {"a2",162},
                {"A3",163},
                {"a3",163},
                {"A4",164},
                {"a4",164},
                {"A5",165},
                {"a5",165},
                {"A6",166},
                {"a6",166},
                {"A7",167},
                {"a7",167},
                {"A8",168},
                {"a8",168},
                {"A9",169},
                {"a9",169},
                {"AA",170},
                {"aa",170},
                {"AB",171},
                {"ab",171},
                {"AC",172},
                {"ac",172},
                {"AD",173},
                {"ad",173},
                {"AE",174},
                {"ae",174},
                {"AF",175},
                {"af",175},
                {"B0",176},
                {"b0",176},
                {"B1",177},
                {"b1",177},
                {"B2",178},
                {"b2",178},
                {"B3",179},
                {"b3",179},
                {"B4",180},
                {"b4",180},
                {"B5",181},
                {"b5",181},
                {"B6",182},
                {"b6",182},
                {"B7",183},
                {"b7",183},
                {"B8",184},
                {"b8",184},
                {"B9",185},
                {"b9",185},
                {"BA",186},
                {"ba",186},
                {"BB",187},
                {"bb",187},
                {"BC",188},
                {"bc",188},
                {"BD",189},
                {"bd",189},
                {"BE",190},
                {"be",190},
                {"BF",191},
                {"bf",191},
                {"C0",192},
                {"c0",192},
                {"C1",193},
                {"c1",193},
                {"C2",194},
                {"c2",194},
                {"C3",195},
                {"c3",195},
                {"C4",196},
                {"c4",196},
                {"C5",197},
                {"c5",197},
                {"C6",198},
                {"c6",198},
                {"C7",199},
                {"c7",199},
                {"C8",200},
                {"c8",200},
                {"C9",201},
                {"c9",201},
                {"CA",202},
                {"ca",202},
                {"CB",203},
                {"cb",203},
                {"CC",204},
                {"cc",204},
                {"CD",205},
                {"cd",205},
                {"CE",206},
                {"ce",206},
                {"CF",207},
                {"cf",207},
                {"D0",208},
                {"d0",208},
                {"D1",209},
                {"d1",209},
                {"D2",210},
                {"d2",210},
                {"D3",211},
                {"d3",211},
                {"D4",212},
                {"d4",212},
                {"D5",213},
                {"d5",213},
                {"D6",214},
                {"d6",214},
                {"D7",215},
                {"d7",215},
                {"D8",216},
                {"d8",216},
                {"D9",217},
                {"d9",217},
                {"DA",218},
                {"da",218},
                {"DB",219},
                {"db",219},
                {"DC",220},
                {"dc",220},
                {"DD",221},
                {"dd",221},
                {"DE",222},
                {"de",222},
                {"DF",223},
                {"df",223},
                {"E0",224},
                {"e0",224},
                {"E1",225},
                {"e1",225},
                {"E2",226},
                {"e2",226},
                {"E3",227},
                {"e3",227},
                {"E4",228},
                {"e4",228},
                {"E5",229},
                {"e5",229},
                {"E6",230},
                {"e6",230},
                {"E7",231},
                {"e7",231},
                {"E8",232},
                {"e8",232},
                {"E9",233},
                {"e9",233},
                {"EA",234},
                {"ea",234},
                {"EB",235},
                {"eb",235},
                {"EC",236},
                {"ec",236},
                {"ED",237},
                {"ed",237},
                {"EE",238},
                {"ee",238},
                {"EF",239},
                {"ef",239},
                {"F0",240},
                {"f0",240},
                {"F1",241},
                {"f1",241},
                {"F2",242},
                {"f2",242},
                {"F3",243},
                {"f3",243},
                {"F4",244},
                {"f4",244},
                {"F5",245},
                {"f5",245},
                {"F6",246},
                {"f6",246},
                {"F7",247},
                {"f7",247},
                {"F8",248},
                {"f8",248},
                {"F9",249},
                {"f9",249},
                {"FA",250},
                {"fa",250},
                {"FB",251},
                {"fb",251},
                {"FC",252},
                {"fc",252},
                {"FD",253},
                {"fd",253},
                {"FE",254},
                {"fe",254},
                {"FF",255},
                {"ff",255}
            };
        }

        glm::u8vec3 ConvHexToVec3(std::string const &hex_str)
        {
            if(hex_str.size() != 7)
            {
                throw ColorInvalid();
            }

            auto r_it =
                    g_lkup_byte_str.find(
                        hex_str.substr(1,2));

            auto g_it =
                    g_lkup_byte_str.find(
                        hex_str.substr(3,2));

            auto b_it =
                    g_lkup_byte_str.find(
                        hex_str.substr(5,2));

            if(r_it == g_lkup_byte_str.end() ||
               g_it == g_lkup_byte_str.end() ||
               b_it == g_lkup_byte_str.end())
            {
                throw ColorInvalid();
            }

            return glm::u8vec3(r_it->second,
                               g_it->second,
                               b_it->second);
        }
    }

    // =========================================================== //
}
