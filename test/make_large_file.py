from random import randint
with open('/tftpboot/largetext.txt', 'w') as f:
    for i in range(1000):
        s = ''
        nc = randint(5, 10)
        for _ in range(nc):
            s = s + chr(ord('a') + randint(0, 25))
        f.write(s + '\n')

