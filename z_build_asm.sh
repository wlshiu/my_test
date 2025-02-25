if [ -z "$1" ]; then
    echo "Usage: $0 <source_file.S>"
    exit 1
fi


SRC_FILE=$1

BASENAME=$(basename "$SRC_FILE" .S)

OBJ_FILE="${BASENAME}.o"
ELF_FILE="${BASENAME}.elf"

riscv64-unknown-elf-as -o $OBJ_FILE $SRC_FILE
riscv64-unknown-elf-ld -o $ELF_FILE $OBJ_FILE
# ./riscv_simulator $ELF_FILE
