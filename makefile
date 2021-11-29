KERNEL_DIR = kernel/
MATELIB_DIR = matelib/

matelib_c :
	$(MAKE) -C $(MATELIB_DIR)

kernel_c :
	$(MAKE) -C $(KERNEL_DIR) 

kernel_r :
	cd $(KERNEL_DIR) && \
	./exec

all : matelib_c kernel_c kernel_r

compile : matelib_c kernel_c