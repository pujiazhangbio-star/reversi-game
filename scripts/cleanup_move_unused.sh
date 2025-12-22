#!/usr/bin/env bash
# 把可能不再需要的文件移动到 archive_unused/ 以便你手动复查后再删除
set -e
ARCHIVE_DIR=archive_unused
mkdir -p "$ARCHIVE_DIR"
# 列出候选项（请在移动前确认）
candidates=(
  "*.vcxproj"
  "*.user"
  "archive/"
  "windows-single-exe.yml"
  "pack-msys2-ucrt64.bat"
)
for p in "${candidates[@]}"; do
  echo "Processing pattern: $p"
  for f in $(ls -A $p 2>/dev/null || true); do
    echo "Moving $f -> $ARCHIVE_DIR/"
    mv "$f" "$ARCHIVE_DIR/"
  done
done

echo "Moved candidate files to $ARCHIVE_DIR. 请检查并确认要永久删除哪些文件。"
