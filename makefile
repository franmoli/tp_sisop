KERNEL_DIR = kernel/
MATELIB_DIR = matelib/

matelib_c :
	$(MAKE) -C $(MATELIB_DIR)

kernel_c :
	$(MAKE) -C $(KERNEL_DIR) 

kernel_r :
	cd $(KERNEL_DIR) && \
	./exec

kernel_r_v :
	cd $(KERNEL_DIR) && \
	./vexec

all : matelib_c kernel_c kernel_r

vexec :	matelib_c kernel_c kernel_r_v

compile : matelib_c kernel_c