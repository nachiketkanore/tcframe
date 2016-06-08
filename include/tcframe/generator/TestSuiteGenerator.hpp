#pragma once

#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "GeneratorLogger.hpp"
#include "GenerationResult.hpp"
#include "TestCaseNameCreator.hpp"
#include "TestCaseGenerationResult.hpp"
#include "TestCaseGenerator.hpp"
#include "tcframe/core.hpp"
#include "tcframe/io.hpp"
#include "tcframe/os.hpp"
#include "tcframe/testcase.hpp"

using std::function;
using std::istringstream;
using std::map;
using std::set;
using std::string;

namespace tcframe {

class TestSuiteGenerator {
private:
    TestCaseGenerator* testCaseGenerator_;
    IOManipulator* ioManipulator_;
    OperatingSystem* os_;
    GeneratorLogger* logger_;

public:
    virtual ~TestSuiteGenerator() {}

    TestSuiteGenerator(
            TestCaseGenerator* testCaseGenerator,
            IOManipulator* ioManipulator,
            OperatingSystem* os,
            GeneratorLogger* logger)
            : testCaseGenerator_(testCaseGenerator)
            , ioManipulator_(ioManipulator)
            , os_(os)
            , logger_(logger)
    {}

    virtual GenerationResult generate(
            const TestSuite& testSuite,
            const CoreConfig& coreConfig) {

        logger_->logIntroduction();

        os_->forceMakeDir(coreConfig.testConfig().testCasesDir());

        map<string, TestCaseGenerationResult> resultsByName;
        generateSampleTests(testSuite.sampleTests(), coreConfig, resultsByName);
        generateOfficialTests(testSuite.officialTests(), coreConfig, resultsByName);
        return GenerationResult(resultsByName);
    }

private:
    void generateSampleTests(
            const vector<SampleTestCase>& sampleTests,
            const CoreConfig& coreConfig,
            map<string, TestCaseGenerationResult>& resultsByName) {

        logger_->logSampleTestCasesIntroduction();

        for (int testCaseId = 1; testCaseId <= sampleTests.size(); testCaseId++) {
            SampleTestCase testCase = sampleTests[testCaseId - 1];
            TestCaseData testCaseData = TestCaseDataBuilder()
                    .setName(TestCaseNameCreator::createSampleTestCaseName(coreConfig.problemConfig().slug(), testCaseId))
                    .setConstraintGroupIds(testCase.constraintGroupIds())
                    .build();

            function<void()> closure = [=] {
                istream* in = new istringstream(testCase.content());
                ioManipulator_->parseInput(in);
            };

            generateTestCase(testCaseData, closure, coreConfig, resultsByName);
        }
    }

    void generateOfficialTests(
            const vector<TestGroup>& officialTests,
            const CoreConfig& coreConfig,
            map<string, TestCaseGenerationResult>& resultsByName) {

        for (const TestGroup& testGroup : officialTests) {
            generateTestGroup(testGroup, coreConfig, resultsByName);
        }
    }

    void generateTestGroup(
            const TestGroup& testGroup,
            const CoreConfig& coreConfig,
            map<string, TestCaseGenerationResult>& resultsByName) {

        logger_->logTestGroupIntroduction(testGroup.id());

        for (int testCaseId = 1; testCaseId <= testGroup.officialTestCases().size(); testCaseId++) {
            OfficialTestCase testCase = testGroup.officialTestCases()[testCaseId - 1];
            string testCaseName = TestCaseNameCreator::createOfficialTestCaseName(
                    coreConfig.problemConfig().slug(),
                    testGroup.id(),
                    testCaseId);
            TestCaseData testCaseData = TestCaseDataBuilder()
                    .setName(testCaseName)
                    .setDescription(testCase.description())
                    .setConstraintGroupIds(testGroup.constraintGroupIds())
                    .build();

            generateTestCase(testCaseData, testCase.closure(), coreConfig, resultsByName);
        }
    }

    TestCaseGenerationResult generateTestCase(
            const TestCaseData& testCaseData,
            const function<void()> testCaseClosure,
            const CoreConfig& coreConfig,
            map<string, TestCaseGenerationResult>& testCaseGenerationResultsByName) {

        logger_->logTestCaseIntroduction(testCaseData.name());

        TestCaseGenerationResult result = testCaseGenerator_->generate(testCaseData, testCaseClosure, coreConfig.testConfig());
        testCaseGenerationResultsByName[testCaseData.name()] = result;

        logger_->logTestCaseGenerationResult(testCaseData.description(), result);

        return result;
    }
};

}