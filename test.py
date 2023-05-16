n = 30
for k in range(n-1):
    p = 1
    k = k+2
    for i in range(k-2):
        i = i+2
        if k % i == 0:
            p = 0
    if p == 1:
        print(k)