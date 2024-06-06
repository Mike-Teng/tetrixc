tmux new-session -s game -d
tmux send-keys -t game './tetrix 192.168.0.158 8888' C-m
tmux attach-session -t game
