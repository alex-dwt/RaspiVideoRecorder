#! /bin/bash
# This file is part of the RaspiVideoRecorder package.
# (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
# For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

FPS=25
TMP_DIR=/tmp/recorder
SOURCE_DIR=$1

function main()
{
	if [ -z "$SOURCE_DIR" ]; then
		printf 'Specify input dir\n'
		exit 1
	fi

	secs=$(ls "$SOURCE_DIR" | sed -e 's/\(.*\)_[0-9]\+.jpg/\1/g' | uniq)

	if [ -z "$secs" ]; then
		printf 'No pictures\n'
		exit 0
	fi

	secs=(${secs//\n/ })
	secsCount=${#secs[@]}

	printf "Founded $secsCount seconds\n\n"

	rm -rf "$TMP_DIR" 2> /dev/null
	mkdir "$TMP_DIR"
	cd "$TMP_DIR"

	local i=0
	for (( ; i < $secsCount; i++ ));
	do
		sec=${secs[$i]}

		printf "Process second #$i: '$sec'\n"

		cd "$TMP_DIR"
		mkdir $i
		cd $i

#		ls $SOURCE_DIR | grep $sec | sort -h
		frames=$(ls -v "$SOURCE_DIR" | grep "$sec" | sed -e 's/.*_\([0-9]\+\).jpg/\1/g')

#		frames=$(ls $SOURCE_DIR | grep $sec | sed -e 's/.*_picture_\(.*\)/\1/g')
		frames=(${frames//\n/ })
		framesCount=${#frames[@]}

		printf "Founded ${#frames[@]} frames\n"

		copyFrames

		#writeText

		increaseFrames

		printf "Second completed\n\n"
	done
}




function copyFrames()
{
	printf "Copy $FPS (or less) frames to temp location\n"

	local i=0
	local num=1
	for (( ; i < $framesCount && i < $FPS; i++ ));
	do
		frame=${frames[$i]}
		cp "$SOURCE_DIR/${sec}_${frame}.jpg" "$num"
		num=$((num + 1))
	done
}




function increaseFrames()
{
	local count=$(( $FPS - $framesCount ))

	if [ "$count" -le 0 ]; then
		return
	fi

	printf "Increase to $FPS frames\n"

	local j=$(( (($count + $framesCount - 1) / $framesCount) ))

	local frame=1
	local next=1
	local total=$framesCount
	local i=0
	while true
	do
		for (( i=0; i < j; i++ ));
		do
			next=${next}_
			cp "$frame" "$next"
			((total++))
			if [ $total -eq $FPS ]; then
				break 2
			fi
		done
		frame=$((frame + 1))
		next=$frame
	done
}




function writeText()
{
	text=$(echo "$sec" | sed -e 's/\(.*\)_\(.*\)_\(.*\)_\(.*\)_\(.*\)_\(.*\)/\1-\2-\3 \4:\5:\6/g')

	printf "Write text \"$text\" on frames\n"

	find . -type f | xargs -n 1 -P 4 -I {} bash -c "convert '{}' -quality 80 -pointsize 56 -fill '#0008' -draw 'rectangle 1345,980,1890,1050' -fill white -draw \"text 1360,1035 '$text'\" '{}'"
}



main
