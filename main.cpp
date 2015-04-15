/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <boost/filesystem.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <regex>


////////////////////////////////////////////////////////////
template<typename T, typename U, typename V, typename W>
int testPattern(const T& pattern, const U& name, const V& target, const W& path)
{
    auto begin = std::regex_iterator<decltype(std::begin(target))>{std::begin(target), std::end(target), pattern};
    auto end = std::regex_iterator<decltype(std::begin(target))>{};

    auto failed = 0;

    for (auto i = begin; i != end; ++i)
    {
        auto match = *i;
        auto line = std::count(std::begin(target), std::begin(target) + match.position(1), '\n') + 1;
        std::cout << path << ":" << line << " " << name << std::endl;
        ++failed;
    }

    return failed;
}


////////////////////////////////////////////////////////////
std::vector<char> getFileContents(const boost::filesystem::path& path)
{
    std::vector<char> contents;

    std::ifstream file(path.string());

    if (!file)
    {
        std::cout << "Failed to open file " << path.string() << std::endl;
        return contents;
    }

    file.seekg(0, std::ios::end);
    contents.resize(file.tellg());
    file.seekg(0, std::ios::beg);

    file.read(contents.data(), contents.size());

    if (contents.empty())
        std::cout << "Empty file: " << path.string() << std::endl;

    return contents;
}


////////////////////////////////////////////////////////////
int lint(const boost::filesystem::path& path)
{
    auto regexFlags = std::regex::ECMAScript | std::regex::optimize;

    const static std::regex sourceExtensionPattern{
        R"(\.(c|cpp|h|hpp|inl|m|mm|frag|vert))", regexFlags
    };

    const static std::regex headerExtensionPattern{
        R"(\.(h|hpp))", regexFlags
    };

    const static std::pair<std::regex, const char*> commonPatterns[] = {
        {std::regex{R"(( |\t)\n)", regexFlags}, "Trailing whitespace"},
        {std::regex{R"((\t))", regexFlags}, "Tab(s)"}
    };

    const static std::pair<std::regex, const char*> sourcePatterns[] = {
        {std::regex{R"(((\n|^)( |\t)*/\*.*\*/\n))", regexFlags}, "C-style comment"},
        {std::regex{R"([^_[:alnum:]](if|while|switch|do)\()", regexFlags}, "Missing space before parenthesis"},
        {std::regex{R"([^_[:alnum:]](if|while|switch|do) \(.*\)\{)", regexFlags}, "Missing space before opening bracket"},
        {std::regex{R"([^_[:alnum:]](if|while|switch|do) \( )", regexFlags}, "Extraneous space after parenthesis"},
        {std::regex{R"([_[:alnum:]](=| =|= )[_[:alnum:]])", regexFlags}, "Missing space around assignment"},
        {std::regex{R"((,[^[ \n]]))", regexFlags}, "Missing space after comma"},
        {std::regex{R"(([^_[:alnum:]]const [_[:alnum:]]+ (&|\*)))", regexFlags}, "Extraneous space before reference/pointer specifier"},
        {std::regex{R"(([^_[:alnum:]]operator[^_ [:alnum:]]+\())", regexFlags}, "Missing space after operator keyword"},
        {std::regex{R"(((== true)|(true ==)|(== false)|(false ==)|(== NULL)|(!= NULL)|(NULL ==)|(NULL !=)))", regexFlags}, "Unnecessary comparison"}
    };

    std::vector<char> contents = getFileContents(path);

    if (contents.empty())
        return 0;

    auto failed = 0;

    for (const auto& pattern : commonPatterns)
        failed += testPattern(pattern.first, pattern.second, contents, path.string());

    if (std::regex_match(path.extension().string(), sourceExtensionPattern))
    {
        if (std::regex_match(path.extension().string(), headerExtensionPattern))
        {

        }

        for (const auto& pattern : sourcePatterns)
            failed += testPattern(pattern.first, pattern.second, contents, path.string());
    }
    else
    {

    }

    return failed;
}


////////////////////////////////////////////////////////////
int check(const boost::filesystem::path& path)
{
    auto regexFlags = std::regex::ECMAScript | std::regex::optimize;

    const static std::regex ignorePatterns[] = {
        // Ignore /extlibs/
        std::regex{R"(.*/extlibs/.*)", regexFlags},

        // Ignore /.git/
        std::regex{R"(.*/\.git/.*)", regexFlags},

        // Ignore binary resource files
        std::regex{R"(.*\.(ogg|wav|flac|ttf|png|jpg|icns|rtf|plist|xib))", regexFlags},

        // Ignore Xcode templates
        std::regex{R"(.*\.xctemplate.*)", regexFlags},

        // Ignore text files
        std::regex{R"(.*(license\.txt|changelog\.txt|readme\.txt))", regexFlags}
    };

    for (const auto& pattern : ignorePatterns)
    {
        if (std::regex_match(path.string(), pattern))
            return 0;
    }

    return lint(path);
}


////////////////////////////////////////////////////////////
int main()
{
    auto failed = 0;

    for (const auto& file : boost::filesystem::recursive_directory_iterator{"."})
    {
        if (boost::filesystem::is_regular_file(file))
            failed += check(file.path().relative_path());
    }

    if (failed)
    {
        std::cout << failed << " errors detected" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
