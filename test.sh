#!/bin/bash

# $1 contient le nom du fichier a traiter
function testFile () {
  declare file=$1
  declare out=${file/txt/out}     # fichier de sortie
  declare dat=${file/txt/dat}     # fichier de donnees
  declare res=${file/txt/res}     # fichier de resultats (sortie validee)
  declare status=OK               # statut OK (ou KO en cas d'erreur)
  declare msg=""                  # message a afficher
  if [[ ! -f $dat ]]; then dat=; fi
  # Suppression du fichier de sortie
  if [[ ($clean || $cleanAll) && -f $out ]]
  then
    echo "Delete $out"
    rm $out
  fi
  # Suppression du fichier de resultats
  if [[ $cleanAll && -f $res ]]
  then
    echo "Delete $res"
    rm $res
  fi
  # Test du fichier et creation du fichier de sortie
  if [[ ! $clean && ! $cleanAll && ( $testAll || ! -f $res ) && -f $file ]]
  then
    echo "Test $file $dat"
    ./tp $verbose $noEval $file $dat >$out 2>&1
    # calcul du message en etudiant le fichier de sortie
    if [[ -f $res ]]
    then
      # il existe un fichier de resultats, on le compare au fichier de sortie
      if [[ `cmp $out $res` ]]
      then
        status=KO
        msg="Unexpected output"
      else
        msg="Validated output"
      fi
    else
      res1=`grep -i -e result -e error $file`     # resultat ou erreur annonce
      res2=`grep -i -e result -e error $out`      # resultat ou erreur produit
      if [[ "$res1" == "" ]]
      then
        # pas de resultat annonce
        if [[ "$res2" =~ [eE][rR][rR][oO][rR] ]]  # je n'ai pas trouve comment ignorer la casse
        then
          status=KO
          msg="$res2"
        else
          msg="Completed $res2"
        fi
      else
        # on compare le resultat annonce et le resultat produit
        if [[ "$res1" == "$res2" ]]
        then
          msg="Validated $res2"
        else
          status=KO
          msg="Unexpected $res2 instead of $res1"
        fi
      fi
    fi
    echo "... $msg"
  fi
  # Validation du fichier de sortie si pas d'erreur ou erreur annoncee
  if [[ $accept && -f $out && $status == OK ]]
  then
    echo "Accept $res"
    mv $out $res
  fi
  # Validation forcee du fichier de sortie
  if [[ $acceptAll && -f $out ]]
  then
    echo "Accept $res"
    mv $out $res
  fi
}

# $1 contient le nom du repertoire a traiter
function testDir () {
  declare dir=$1
  echo "Directory $dir"
  for file in $dir/*.txt
  do
    if [[ -f $file ]]
    then
      testFile $file
    fi
  done
  if [[ $recurse ]]
  then
    for dir2 in $dir/*
    do
      if [[ -d $dir2 ]]
      then
        testDir $dir2
      fi
    done
  fi
  echo "... done $dir"
}

# $1 contient le code de retour
# $2 contient le message d'erreur
function help () {
  if [[ $2 ]]; then echo $2; fi
  echo "test.sh -h[elp]                                 # command description  "
  echo "test.sh [option...] file_or_dir...              # test files and directories"
  echo "option -r(recursive) : recursively browse directories"
  echo "       -t(est)       : execute test file unless a result file exists (default)"
  echo "       -T(est)       : execute test file even if a result file exists"
  echo "       -c(lean)      : remove output file associated to test file"
  echo "       -C(lean)      : remove output and result files associated to test file"
  echo "       -a(ccept)     : move output file to result file unless an error is detected"
  echo "       -A(ccept)     : move output file to result file even if an error is detected"
  echo "       -v(erbose)    : set mode Verbose 'on'"
  echo "       -e(noEval)    : prevent from evaluating expressions"
  exit 0
}

while [[ $# > 0 ]]; do
  if [[ $1 =~ -h ]]; then help 0
  elif [[ $1 =~ -r ]]; then recurse=1
  elif [[ $1 =~ -t ]]; then test=1
  elif [[ $1 =~ -T ]]; then testAll=1
  elif [[ $1 =~ -c ]]; then clean=1
  elif [[ $1 =~ -C ]]; then cleanAll=1
  elif [[ $1 =~ -a ]]; then accept=1
  elif [[ $1 =~ -A ]]; then acceptAll=1
  elif [[ $1 =~ -v ]]; then verbose=-v
  elif [[ $1 =~ -e ]]; then noEval=-e
  elif [[ -d $1 ]]; then testDir $1
  elif [[ -f $1 && $1 =~ .*txt ]]; then testFile $1
  else help 1 "Bad argument : $1"
  fi
  shift
done