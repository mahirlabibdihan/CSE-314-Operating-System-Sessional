#!/bin/bash
submission_dir=$1
target_dir=$2
test_dir=$3
answer_dir=$4
verbose=0
noexecute=0

# Associative Array
extToLang=(["c"]="C" ["py"]="Python" ["java"]="Java")
extToSrc=(["c"]="main" ["py"]="main" ["java"]="Main")

compile_C() {
    gcc "$1/$2.c" -o "$1/$2.out"
}

run_C() {
    "$1/$2.out" <$3 >$4
}

compile_Java() {
    javac "$1/$2.java"
}

run_Java() {
    java -cp $1 $2 <$3 >$4
}

run_Python() {
    python3 "$1/$2.py" <$3 >$4
}

run() {
    case $5 in
    "C") run_C $1 $2 $3 $4 ;;
    "Java") run_Java $1 $2 $3 $4 ;;
    "Python") run_Python $1 $2 $3 $4 ;;
    esac
}

compile() {
    case $3 in
    "C") compile_C $1 $2 ;;
    "Java") compile_Java $1 $2 ;;
    "Python") ;;
    esac
}

match() {
    matched=0
    notmatched=0

    compile $src_dir $source $lang
    for t in $test_dir/*; do
        base=$(basename ${t%.*})
        no=${base:4}
        run $src_dir $source $t "$src_dir/out$no.txt" $lang
        check=$(diff "$src_dir/out$no.txt" "$answer_dir/ans$no.txt")
        if [ -z "$check" ]; then
            matched=$(expr $matched + 1)
        else
            notmatched=$(expr $notmatched + 1)
        fi
    done
    echo "$sid,$lang,$matched,$notmatched" >>"$target_dir/result.csv"
}

main() {
    echo "Found $(find $test_dir -type f | wc -l) test files"
    rm -r targets
    mkdir targets
    echo "student_id,type,matched,not_matched" >"$target_dir/result.csv"
    for file in submissions/*.zip; do
        sid=${file: -11:-4}
        if [ $verbose = 1 ]; then
            echo "Organizing files of $sid"
        fi

        # Getting source file name
        file_path=$(unzip -Z -1 "$file" | grep -E '\.(c|py|java)$')

        ext="${file_path##*.}"
        lang=${extToLang[$ext]}
        source=${extToSrc[$ext]}

        src_dir="$target_dir/$lang/$sid"
        mkdir -p $src_dir

        # Copying source file to targets/sid/
        unzip -j "$file" "$file_path" -d $src_dir >/dev/null 2>&1 && mv "$src_dir/$(basename "$file_path")" $src_dir/$source.$ext >/dev/null 2>&1

        # Execute source files
        if [ $noexecute = 0 ]; then
            if [ $verbose = 1 ]; then
                echo "Executing files of $sid"
            fi
            match
        fi
    done
}

# Setting flags
if [ $# -gt 4 ]; then
    if [ $5 = "-v" ]; then
        verbose=1
    elif [ $5 = "-noexecute" ]; then
        noexecute=1
    fi
fi

if [ $# -gt 5 ]; then
    if [ $6 = "-v" ]; then
        verbose=1
    elif [ $6 = "-noexecute" ]; then
        noexecute=1
    fi
fi

main
