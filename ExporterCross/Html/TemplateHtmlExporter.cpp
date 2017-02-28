// OpenCppCoverage is an open source code coverage for C++.
// Copyright (C) 2014 OpenCppCoverage
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <ctemplate/template_dictionary.h>
#include <tests/template_test_util.h>

#include "TemplateHtmlExporter.hpp"

#include "Tools/Tool.hpp"

#include <CppCoverageCross/CoverageRate.hpp>
#include <CppCoverageCross/CoverageData.hpp>
#include <CppCoverageCross/ModuleCoverage.hpp>
#include <CppCoverageCross/FileCoverage.hpp>

namespace fs = boost::filesystem;

namespace Exporter
{
    namespace
    {
        const std::string coverRateTemplate = "COVER_RATE";
        const std::string uncoverRateTemplate = "UNCOVER_RATE";
        const std::string codeTemplate = "CODE";
        const std::string messageTemplate = "MAIN_MESSAGE";
        const std::string idTemplate = "ID";
        const std::string thirdPartyPathTemplate = "THIRD_PARTY_PATH";

        //-------------------------------------------------------------------------
        std::string ToStr(const std::wstring &str)
        {
            return Tools::ToUtf8String(str);
        }

        //-------------------------------------------------------------------------
        std::string GetUuid()
        {
            boost::uuids::random_generator generator;
            boost::uuids::uuid id(generator());

            return boost::uuids::to_string(id);
        }

        //-------------------------------------------------------------------------
        void FillSection(
                ctemplate::TemplateDictionary &sectionDictionary,
                const fs::path *link,
                const CppCoverage::CoverageRate &coverageRate,
                const std::string &name)
        {
            if (link)
            {
                sectionDictionary.SetValueAndShowSection(TemplateHtmlExporter::LinkTemplate,
                                                         link->string(),
                                                         TemplateHtmlExporter::ItemLinkSection);
            }
            else
            {
                sectionDictionary.ShowSection(TemplateHtmlExporter::ItemNoLinkSection);
            }

            sectionDictionary.SetIntValue(coverRateTemplate, coverageRate.GetPercentRate());
            sectionDictionary.SetIntValue(uncoverRateTemplate, 100 - coverageRate.GetPercentRate());
            sectionDictionary.SetIntValue(TemplateHtmlExporter::ExecutedLineTemplate,
                                          coverageRate.GetExecutedLinesCount());
            sectionDictionary.SetIntValue(TemplateHtmlExporter::UnExecutedLineTemplate,
                                          coverageRate.GetUnExecutedLinesCount());
            sectionDictionary.SetIntValue(TemplateHtmlExporter::TotalLineTemplate,
                                          coverageRate.GetTotalLinesCount());
            sectionDictionary.SetValue(idTemplate, GetUuid());
            sectionDictionary.SetValue(TemplateHtmlExporter::NameTemplate, name);
        }

        //-------------------------------------------------------------------------
        void WriteContentTo(const std::string &content, const fs::path &path)
        {
            std::ofstream ofs(path.string());

            if (!ofs)
                throw std::runtime_error("Cannot open file" + path.string());
            ofs << content;
            ofs.flush();
        }

        //-------------------------------------------------------------------------
        std::string GenerateTemplate(
                const ctemplate::TemplateDictionary &templateDictionary,
                const fs::path &templatePath)
        {
            std::string output;

            if (!ctemplate::ExpandTemplate(templatePath.string(), ctemplate::DO_NOT_STRIP,
                                           &templateDictionary, &output))
            {
                throw std::runtime_error("Cannot generate output for " + templatePath.string());
            }

            return output;
        }

        //---------------------------------------------------------------------
        std::vector<const ctemplate::TemplateDictionary *> GetSectionDictionaries(
                const ctemplate::TemplateDictionaryPeer &peer,
                const std::string &sectionName)
        {
            std::vector<const ctemplate::TemplateDictionary *> templateDictionaries;
            peer.GetSectionDictionaries(sectionName, &templateDictionaries);

            return templateDictionaries;
        }

        //-------------------------------------------------------------------------
        void CheckLinkExists(
                const ctemplate::TemplateDictionary &templateDictionary,
                const std::string &sectionName,
                const fs::path &output)
        {
            ctemplate::TemplateDictionaryPeer rootPeer(&templateDictionary);
            auto dictionaries = GetSectionDictionaries(rootPeer, sectionName);

            for (const auto &dictionary : dictionaries)
            {
                ctemplate::TemplateDictionaryPeer peer(dictionary);
                auto itemLinkDictionaries =
                        GetSectionDictionaries(peer, TemplateHtmlExporter::ItemLinkSection);

                if (!itemLinkDictionaries.empty())
                {
                    ctemplate::TemplateDictionaryPeer itemLinkPeer(itemLinkDictionaries[0]);
                    auto linkStr = itemLinkPeer.GetSectionValue(TemplateHtmlExporter::LinkTemplate);

                    if (linkStr)
                    {
                        auto fullPath = output.parent_path() / linkStr;

                        if (!fs::exists(fullPath))
                        {
                            throw std::runtime_error("Link: " + fullPath.string() + " does not exists");
                        }
                    }
                }
            }
        }

        //-------------------------------------------------------------------------
        void WriteTemplate(
                const ctemplate::TemplateDictionary &templateDictionary,
                const fs::path &templatePath,
                const fs::path &output)
        {
            std::string content = GenerateTemplate(templateDictionary, templatePath);
            WriteContentTo(content, output);
        }
    }

    //-------------------------------------------------------------------------
    const std::string TemplateHtmlExporter::MainTemplateItemSection = "ITEMS";
    const std::string TemplateHtmlExporter::TitleTemplate = "TITLE";
    const std::string TemplateHtmlExporter::ExecutedLineTemplate = "EXECUTED_LINE";
    const std::string TemplateHtmlExporter::UnExecutedLineTemplate = "UNEXECUTED_LINE";
    const std::string TemplateHtmlExporter::LinkTemplate = "LINK";
    const std::string TemplateHtmlExporter::TotalLineTemplate = "TOTAL_LINE";
    const std::string TemplateHtmlExporter::NameTemplate = "NAME";
    const std::string TemplateHtmlExporter::ItemLinkSection = "ITEM_LINK";
    const std::string TemplateHtmlExporter::ItemNoLinkSection = "ITEM_NO_LINK";

    //-------------------------------------------------------------------------
    TemplateHtmlExporter::TemplateHtmlExporter(const fs::path &templateFolder)
            : mainTemplatePath_(templateFolder / "MainTemplate.html"),
              fileTemplatePath_(templateFolder / "SourceTemplate.html")
    {
    }

    //-------------------------------------------------------------------------
    std::unique_ptr<ctemplate::TemplateDictionary>
    TemplateHtmlExporter::CreateTemplateDictionary(
            const std::wstring &title,
            const std::wstring &message) const
    {
        std::string titleStr{ToStr(title)};
        std::unique_ptr<ctemplate::TemplateDictionary> dictionary;

        dictionary.reset(new ctemplate::TemplateDictionary(titleStr));

        dictionary->SetValue(TitleTemplate, titleStr);
        dictionary->SetValue(messageTemplate, ToStr(message));

        return dictionary;
    }

    //-------------------------------------------------------------------------
    void TemplateHtmlExporter::AddFileSectionToDictionary(
            const std::string &originalFilename,
            const CppCoverage::CoverageRate &coverageRate,
            const fs::path *fileOutput,
            ctemplate::TemplateDictionary &moduleTemplateDictionary) const
    {
        auto sectionDictionary = moduleTemplateDictionary.AddSectionDictionary(
                MainTemplateItemSection);

        moduleTemplateDictionary.SetValue(thirdPartyPathTemplate, "../third-party");
        FillSection(*sectionDictionary, fileOutput, coverageRate, originalFilename);
    }

    //-------------------------------------------------------------------------
    void TemplateHtmlExporter::AddModuleSectionToDictionary(
            const std::string &originalFilename,
            const CppCoverage::CoverageRate &coverageRate,
            const fs::path &moduleOutput,
            ctemplate::TemplateDictionary &projectDictionary) const
    {
        auto sectionDictionary = projectDictionary.AddSectionDictionary(MainTemplateItemSection);

        projectDictionary.SetValue(thirdPartyPathTemplate, "third-party");
        FillSection(*sectionDictionary, &moduleOutput, coverageRate, originalFilename);
    }

    //-------------------------------------------------------------------------
    void TemplateHtmlExporter::GenerateModuleTemplate(
            const ctemplate::TemplateDictionary &templateDictionary,
            const fs::path &output) const
    {
        CheckLinkExists(templateDictionary, MainTemplateItemSection, output);
        WriteTemplate(templateDictionary, mainTemplatePath_, output);
    }

    //-------------------------------------------------------------------------
    void TemplateHtmlExporter::GenerateProjectTemplate(
            const ctemplate::TemplateDictionary &templateDictionary,
            const fs::path &output) const
    {
        CheckLinkExists(templateDictionary, MainTemplateItemSection, output);
        WriteTemplate(templateDictionary, mainTemplatePath_, output);
    }

    //-------------------------------------------------------------------------
    void TemplateHtmlExporter::GenerateSourceTemplate(
            const std::wstring &title,
            const std::wstring &codeContent,
            const fs::path &output) const
    {
        auto titleStr = Tools::ToUtf8String(title);
        ctemplate::TemplateDictionary dictionary(titleStr);

        dictionary.SetValue(TitleTemplate, titleStr);
        dictionary.SetValue(codeTemplate, Tools::ToUtf8String(codeContent));
        WriteTemplate(dictionary, fileTemplatePath_, output);
    }
}
