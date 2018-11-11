from PIL import Image

seq = []

for i in range(13):
    seq.append(Image.open('./res/{}.bmp'.format(i)))

seq[0].save('./res/res.gif', save_all=True,
            append_images=seq[1:], duration=0.1, loop=0)
