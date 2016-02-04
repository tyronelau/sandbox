#! /bin/bash
function subst() {
  printf "Entering directory: $1\n"

  if (( $# < 3 )) ; then
    printf "Error: only $# argument supplied\n"
    return -1
  fi
  local files=`ls $1`
  cd $1
  local f new
  for f in $files; do
    printf "file/directory: $f\n"
    if [[ -f $f ]]; then
      new=${f//$2/$3}
      if [[ $new != $f ]]; then
        mv $f $new
      fi
    elif [[ -d $f ]]; then
      new=${f//$2/$3}
      printf "before: $f, after: $new\n"
      subst $f $2 $3
      printf "2 before: $f, after: $new\n"
      if [[ $new != $f ]]; then
        mv $f $new
      fi
    fi
  done
  cd ..
  printf "Leaving directory: $1\n"
}

subst test Soma Aggregator
