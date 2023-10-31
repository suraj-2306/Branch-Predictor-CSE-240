import math

# for k in range(11,18):
#     for y in range(5,103):
#         for x in range(11,19):
#             result = pow(2,k+2)+pow(2,x)*y
#             if(result<pow(2,17)):
#                 print("#", y, x, k, "->")
for y in range(5,103):
    for x in range(11,19):
        for k in range(11,18):
            result = pow(2,y+2)+pow(2,k)*x+pow(2,x)*2
            if(result<pow(2,17)):
                print("#", y, x, k, "->")
