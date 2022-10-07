from tomark import Tomark
import re
import os

SUMMARY_FILE = os.getenv('GITHUB_STEP_SUMMARY')

saved_lines = []
failed_tests_str = []

with open('log/latest_test/stdout_stderr.log', 'r') as file:
    for line in reversed(file.readlines()):
        saved_lines.append(line)
        if (re.search('.*The following tests FAILED:.*', line)):
            break

# Exit if no test failed
if (not saved_lines):
    exit(0)

for i, test in enumerate(saved_lines):
    s = re.search('\d* - .* \(Failed\)', test)
    if (s):
        failed_tests_str.insert(0, s.group())

failed_tests = []
for test in failed_tests_str:
    s = re.findall('\S+', test)
    failed_tests.append(dict({"ID": s[0], "Name": s[2]}))

print(Tomark.table(failed_tests))
with open(SUMMARY_FILE, "a") as file:

    file.write(f'\n{Tomark.table(failed_tests)}')
