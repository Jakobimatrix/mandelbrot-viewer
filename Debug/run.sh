if [ -e src/executables/mandelbroetchen_start ]; then
    exec src/executables/mandelbroetchen_start
else
    echo "Cannot find executable."
fi
