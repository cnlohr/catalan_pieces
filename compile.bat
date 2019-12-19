rem cat catalan.c | gzip -9 | base64 -w 10000 | qrencode 
c:\tcc\tcc catalan.c -o catalan.exe
catalan.exe > catalan.svg
