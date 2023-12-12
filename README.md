# push stream
```bash
ffmpeg -re -i lizi.flv -c copy -f flv -y rtmp://localhost/live/livestream
```