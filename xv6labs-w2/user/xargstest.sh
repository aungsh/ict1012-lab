mkdir a
echo hello > a/b
mkdir c
echo hello > c/b
echo hello > b
echo a/b c/b b > list
cat list | xargs grep hello