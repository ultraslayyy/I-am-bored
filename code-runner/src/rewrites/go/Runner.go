// https://github.com/ultraslayyy/I-am-bored/tree/info/code-runner/src/rewrites/runner.go
// Please don't delete the above line, to credit me. But I can't stop you.
// runner version: 0.1.0
package runner

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"time"
)

type TestCase struct {
	Input    string
	Expected string
}

type TestCaseOptions struct {
	Parallel    bool
	Concurrency int
	Cases       []TestCase
}

type RunResult struct {
	Stdout   string
	Stderr   string
	ExitCode int
	Success  bool
	TimeMs   float64
	MemoryKb int
}

type TestCaseResult struct {
	RunResult
	Input    string
	Expected string
	Passed   bool
}

type LangConfig struct {
	Ext         string
	Compile     string
	Run         string
	DockerImage string
}

var LANGS = map[string]LangConfig{
	"go":         {Ext: ".go", Run: "go run {file}", DockerImage: "golang:1.22"},
	"python":     {Ext: ".py", Run: "python3 {file}", DockerImage: "python:2.7.18"},
	"javascript": {Ext: ".js", Run: "node {file}", DockerImage: "node:20"},
}

var CustomContainers = map[string]string{
	"typescript": "runner_typescript",
	"pandas":     "runner_pandas",
	"kotlin":     "runner_kotlin",
	"ms sql":     "runner_mssql",
	"oracle":     "runner_oracle",
}

func formatCmd(template, filePath string) string {
	base := strings.TrimSuffix(filePath, filepath.Ext(filePath))
	cmd := strings.ReplaceAll(template, "{file}", filePath)
	cmd = strings.ReplaceAll(cmd, "{base}", base)
	return cmd
}

func RunCodeLocal(code, language string, testCases *TestCaseOptions, filenamePrefix string, input string) ([]TestCaseResult, RunResult, error) {
	lang, ok := LANGS[language]
	if !ok {
		return nil, RunResult{}, fmt.Errorf("unsupported language: %s", language)
	}

	tempDir := filepath.Join(".", "temp")
	os.MkdirAll(tempDir, os.ModePerm)
	uniqueId := fmt.Sprintf("%d-%s", time.Now().UnixNano(), randomString(6))
	filePath := filepath.Join(tempDir, fmt.Sprintf("%s-%s%s", filenamePrefixOrDefault(filenamePrefix), uniqueId, lang.Ext))
	ioutil.WriteFile(filePath, []byte(code), 0644)

	// Compile if necessary
	if lang.Compile != "" {
		compileCmd := formatCmd(lang.Compile, filePath)
		compileRes, err := execCommand(compileCmd, "")
		if err != nil || compileRes.ExitCode != 0 {
			if testCases != nil {
				return []TestCaseResult{{
					RunResult: compileRes,
					Input:     "",
					Passed:    false,
				}}, RunResult{}, nil
			}
			return nil, compileRes, nil
		}
	}

	runCmd := formatCmd(lang.Run, filePath)

	if testCases != nil {
		results := make([]TestCaseResult, len(testCases.Cases))
		limit := 10
		if testCases.Concurrency > 0 {
			limit = testCases.Concurrency
		}

		if testCases.Parallel {
			var wg sync.WaitGroup
			sem := make(chan struct{}, limit)
			for i, tc := range testCases.Cases {
				wg.Add(1)
				go func(i int, tc TestCase) {
					defer wg.Done()
					sem <- struct{}{}
					res, _ := execCommand(runCmd, tc.Input)
					<-sem
					cleanedOut := strings.TrimSpace(res.Stdout)
					expected := strings.TrimSpace(tc.Expected)
					results[i] = TestCaseResult{
						RunResult: res,
						Input:     tc.Input,
						Expected:  tc.Expected,
						Passed:    expected != "" && cleanedOut == expected,
					}
				}(i, tc)
			}
			wg.Wait()
		} else {
			for i, tc := range testCases.Cases {
				res, _ := execCommand(runCmd, tc.Input)
				cleanedOut := strings.TrimSpace(res.Stdout)
				expected := strings.TrimSpace(tc.Expected)
				results[i] = TestCaseResult{
					RunResult: res,
					Input:     tc.Input,
					Expected:  tc.Expected,
					Passed:    expected != "" && cleanedOut == expected,
				}
			}
		}
		return results, RunResult{}, nil
	} else {
		res, _ := execCommand(runCmd, input)
		return nil, res, nil
	}
}

func execCommand(command, input string) (RunResult, error) {
	start := time.Now()
	cmd := exec.Command("bash", "-c", command)

	if input != "" {
		cmd.Stdin = strings.NewReader(input)
	}

	var stdout, stderr bytes.Buffer
	cmd.Stdout = &stdout
	cmd.Stderr = &stderr

	err := cmd.Run()
	duration := time.Since(start)

	memKb := runtime.MemStats{}.Alloc / 1024

	exitCode := 0
	if exitErr, ok := err.(*exec.ExitError); ok {
		exitCode = exitErr.ExitCode()
	}

	return RunResult{
		Stdout:   stdout.String(),
		Stderr:   stderr.String(),
		ExitCode: exitCode,
		Success:  err == nil,
		TimeMs:   float64(duration.Milliseconds()),
		MemoryKb: int(memKb),
	}, err
}

func filenamePrefixOrDefault(prefix string) string {
	if prefix == "" {
		return "run"
	}
	return prefix
}

func randomString(n int) string {
	letters := []rune("abcdefghijklmnopqrstuvwxyz0123456789")
	b := make([]rune, n)
	for i := range b {
		b[i] = letters[time.Now().UnixNano()%int64(len(letters))]
	}
	return string(b)
}

func RunCodeDocker(code, language string, testCases *TestCaseOptions, filenamePrefix string, input string) ([]TestCaseResult, RunResult, error) {
	lang, ok := LANGS[language]
	if !ok {
		return nil, RunResult{}, fmt.Errorf("unsupported language: %s", language)
	}

	codeDir := filepath.Join(".", "code")
	os.MkdirAll(codeDir, os.ModePerm)
	uniqueId := fmt.Sprintf("%d-%s", time.Now().UnixNano(), randomString(6))
	filePath := filepath.Join(codeDir, fmt.Sprintf("%s-%s%s", filenamePrefixOrDefault(filenamePrefix), uniqueId, lang.Ext))
	ioutil.WriteFile(filePath, []byte(code), 0644)

	isCustom := false
	containerOrImage := lang.DockerImage
	if val, exists := CustomContainers[strings.ToLower(language)]; exists {
		isCustom = true
		containerOrImage = val
	}

	runCmd := formatCmd(lang.Run, "/app/"+filepath.Base(filePath))
	dockerCmd := ""
	if isCustom {
		dockerCmd = fmt.Sprintf(`docker exec -i %s bash -c "echo \"$INPUT\" | %s"`, containerOrImage, runCmd)
	} else {
		dockerCmd = fmt.Sprintf(`docker run --rm --network none --cpus=.5 -m 256m -v "%s:/app" -w /app %s bash -c "echo \"$INPUT\" | %s"`, codeDir, containerOrImage, runCmd)
	}

	execDocker := func(input string) RunResult {
		res, _ := execCommand(fmt.Sprintf("INPUT=%s %s", escapeInput(input), dockerCmd), "")
		return formatDockerTimeMem(res)
	}

	if testCases != nil {
		results := make([]TestCaseResult, len(testCases.Cases))
		limit := 10
		if testCases.Concurrency > 0 {
			limit = testCases.Concurrency
		}

		if testCases.Parallel {
			var wg sync.WaitGroup
			sem := make(chan struct{}, limit)
			for i, tc := range testCases.Cases {
				wg.Add(1)
				go func(i int, tc TestCase) {
					defer wg.Done()
					sem <- struct{}{}
					res := execDocker(tc.Input)
					<-sem
					cleanedOut := strings.TrimSpace(res.Stdout)
					expected := strings.TrimSpace(tc.Expected)
					results[i] = TestCaseResult{
						RunResult: res,
						Input:     tc.Input,
						Expected:  tc.Expected,
						Passed:    expected != "" && cleanedOut == expected,
					}
				}(i, tc)
			}
			wg.Wait()
		} else {
			for i, tc := range testCases.Cases {
				res := execDocker(tc.Input)
				cleanedOut := strings.TrimSpace(res.Stdout)
				expected := strings.TrimSpace(tc.Expected)
				results[i] = TestCaseResult{
					RunResult: res,
					Input:     tc.Input,
					Expected:  tc.Expected,
					Passed:    expected != "" && cleanedOut == expected,
				}
			}
		}
		return results, RunResult{}, nil
	} else {
		res := execDocker(input)
		return nil, res, nil
	}
}

func formatDockerTimeMem(res RunResult) RunResult {
	timeMs := res.TimeMs
	memoryKb := res.MemoryKb

	for _, part := range strings.Fields(res.Stderr) {
		if strings.HasPrefix(part, "TIME:") {
			fmt.Sscanf(part, "TIME:%f", &timeMs)
			timeMs *= 1000 // convert to ms
		} else if strings.HasPrefix(part, "MEM:") {
			fmt.Sscanf(part, "MEM:%d", &memoryKb)
		}
	}

	res.TimeMs = timeMs
	res.MemoryKb = memoryKb
	return res
}

func escapeInput(input string) string {
	return strings.ReplaceAll(input, `"`, `\"`)
}
