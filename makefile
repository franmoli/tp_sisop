KERNEL_DIR = kernel/
MEMORY_DIR = memoria/
MATELIB_DIR = matelib/
SWAP_DIR = swap/
SHARED_DIR = shared/

matelib_c :
	$(MAKE) -C $(MATELIB_DIR)

kernel_c :
	$(MAKE) -C $(KERNEL_DIR) 

mem_c :
	$(MAKE) -C $(MEMORY_DIR) 

swap_c :
	$(MAKE) -C $(SWAP_DIR) 

kernel_r :
	cd $(KERNEL_DIR) && \
	./exec

kernel_r_c :
	cd $(KERNEL_DIR) && \
	gnome-terminal -x sh -c "./exec; bash"

kernel_r_v :
	cd $(KERNEL_DIR) && \
	./vexec

kernel_r_v_c :
	cd $(KERNEL_DIR) && \
	gnome-terminal -x sh -c "./vexec; bash"

mem_r : 
	cd $(MEMORY_DIR) && \
	./exec

mem_r_c : 
	cd $(MEMORY_DIR) && \
	gnome-terminal -x sh -c "./exec; bash"

mem_r_v : 
	cd $(MEMORY_DIR) && \
	./vexec

mem_r_v_c : 
	cd $(MEMORY_DIR) && \
	gnome-terminal -x sh -c "./vexec; bash"

swap_r : 
	cd $(SWAP_DIR) && \
	./exec

swap_r_c : 
	cd $(SWAP_DIR) && \
	gnome-terminal -x sh -c "./exec; bash"

swap_r_v : 
	cd $(SWAP_DIR) && \
	./vexec

swap_r_v_c : 
	cd $(SWAP_DIR) && \
	gnome-terminal -x sh -c "./vexec; bash"

all : compile swap_r_c mem_r_c kernel_r_c

vall : compile swap_r_v_c mem_r_v_c kernel_r_v_c

vexec :	compile kernel_r_v

compile : matelib_c kernel_c mem_c swap_c

mem: compile mem_r

swap: compile swap_r

deploy_obj: 
	cd $(SWAP_DIR) && \
	mkdir obj
	cd $(MEMORY_DIR) && \
	mkdir obj
	cd $(KERNEL_DIR) && \
	mkdir obj
	cd $(MATELIB_DIR) && \
	mkdir obj
	cd $(SHARED_DIR) && \
	mkdir obj

deploy: deploy_obj compile
