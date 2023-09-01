for i in $(seq -w 00 65);
do
	#convert LYN${i}.png -crop 97x54+38+62 -background none cropped/LYN${i}.png
	convert LYN${i}.png -crop 57x76+85+45 -background none cropped_2/LYN${i}.png
done
