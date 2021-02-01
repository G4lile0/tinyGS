import subprocess

revision = (
    subprocess.check_output(["git", "describe", "--tags", "--always"])
    .strip()
    .decode("utf-8")
)
print("-DGIT_VERSION='\"%s\"'" % revision)