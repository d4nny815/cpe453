def prob_2(virt_addr, page_frame):
    phys_addr = (virt_addr & ((1 << 12) - 1)) + page_frame 
    print(phys_addr)
    return

def main():
    prob_2(20, 1024 * 8)
    prob_2(4100, 1024 * 4)
    prob_2(8300, 1024 * 24)

main()