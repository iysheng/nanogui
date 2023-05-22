/*
    src/translate_backend.cpp -- C++ version of an trslate backend application that shows
    how to use the various widget classes.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "translate.h"
#include <thread>
#include <unordered_map>


//#define DICT_FILE     "/home/red/Projects/Notes/English.md"
#define DICT_FILE     "/tmp/English.md"
static FILE *gs_dict_filep;
static std::unordered_map<std::string, std::string> gs_dic_unorderd_map;

static int dict_analysis_markdown(std::FILE *const filep)
{
#define LINE_CHAR_MAX_COUNTS    256
    char *line_str = nullptr;
    size_t  line_char_counts = LINE_CHAR_MAX_COUNTS;
    int     line_counts = 0;
    std::size_t english_pos, chinese_pos;

    while(getline((char **)&line_str, &line_char_counts, filep) != -1)
    {
        printf("%d:%s\n", line_counts, line_str);
        line_counts++;
        std::string line_str_template(line_str);
        english_pos = line_str_template.find('|', 1);
        chinese_pos = line_str_template.find('|', english_pos + 1);
        printf("%d %d\n", english_pos, chinese_pos);
        *(line_str + english_pos) = '\x00';
        *(line_str + chinese_pos) = '\x00';
        printf("analysis:[%s]->[%s]\n", line_str + 1, line_str + english_pos + 1);
        gs_dic_unorderd_map.insert({line_str + 1, line_str + english_pos + 1});
#if 0
        free(line_str);
        line_str = nullptr;
#endif
    }

    return line_counts;
}

std::string *find_in_dict_datasets(const char *name)
{
    auto ans = gs_dic_unorderd_map.find(name);
    std::string null("");

    if (ans != gs_dic_unorderd_map.end())
        return &ans->second;
    else
        return nullptr;
}

void parse_dicfile_thread(RedBurntool *app)
{
    gs_dict_filep = fopen(DICT_FILE, "r");
    if (!gs_dict_filep)
    {
        printf("Invalid file:%s, err=%d", DICT_FILE, errno);
        return;
    }

    int ret = dict_analysis_markdown(gs_dict_filep);

    fclose(gs_dict_filep);
}
