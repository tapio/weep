#!/bin/bash

THISPATH="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
FILE="$THISPATH/../data/pbrtestscene.json"

cat << EOF > "$FILE"
{
	"include": [ "debugprefabs.json" ],
	"environment": {
		"sunColor": [ 0.3, 0.3, 0.3 ],
		"sunPosition": [ 1, 1, 1 ]
	},
	"prefabs": {
		"matsphere": {
			"material": {
				"ambient": [ 0.01, 0.01, 0.01 ]
			},
			"geometry": [
				{ "debug/sphere_hi.obj": 5.0 },
				{ "debug/sphere.obj": 20.0 },
				{ "debug/sphere_lo.obj": 1e999 }
			],
			"scale": 0.4
		}
	},
	"objects": [
	{
		"name": "camera",
		"geometry": "debug/cube.obj",
		"body": {
			"shape": "capsule",
			"mass": 100,
			"angularFactor": 0,
			"noSleep": true,
			"noGravity": true
		},
		"scale": [ 0.4, 1.5, 0.4 ],
		"position": [0, 0.75, 0]
	},{
		"prefab": "plane",
		"material": { "uvRepeat": 20 },
		"position": [ 0, -0.5, 0 ],
		"scale": [ 50, 0.01, 50 ],
		"body": { "mass": 0, "shape": "box" }
	},{
EOF

X=0
Y=0
S=5
D=1

function divide() {
	echo $(( 100 * $1 / ($2 - 1) )) | sed -e 's/..$/.&/;t' -e 's/.$/.0&/'
}

for i in {1..25}; do
	cat << EOF >> "$FILE"
		"prefab": "matsphere",
		"material": {
			"metalness": $(divide $X $S),
			"roughness": $(divide $Y $S)
		},
		"position": [ $(($X*$D)), $(($Y*$D)), -2 ]
	},{
EOF
	if [ $X -ge $(($S-1)) ]; then
		X=0
		Y=$(($Y+1))
	else
		X=$(($X+1))
	fi
done

CONTENTS=`head --lines=-1 "$FILE"`
echo "$CONTENTS" > "$FILE"
echo -e "\t}\n]}" >> "$FILE"
# Fix floats
sed -i 's/\": \./\": 0\./' "$FILE"
