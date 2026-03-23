#include <iostream>
#include <vector>
#include <chrono>
#include <memory>

// n: the block_size to be aligned; align: the alignment boundary
static inline std::size_t align_up(std::size_t n, std::size_t align){
    return (n + (align - 1)) & ~(align - 1);
}

class FixedSizePool{
private:
    struct Node{
        Node* next;
    };
    Node* free_list_;           // list head of free blocks
    std::size_t block_size_;
    std::size_t blocks_per_page_;
    std::vector<void*> pages;   // pages that have been allocated

    // request for one page from the system, spilts it into blocks and add them to the list
    void expand(){
        std::size_t page_size = block_size_ * blocks_per_page_;
        char* page = static_cast<char*>(::operator new[](page_size));
        pages.push_back(page);
        // split page into blocks
        for(std::size_t i = 0; i < blocks_per_page_; ++i){
            char* addr = page + i * block_size_;
            Node* node = reinterpret_cast<Node*>(addr);
            node->next = free_list_;
            free_list_ = node;
        }
    }

    std::size_t adjust_block_size(std::size_t block_size){
        std::size_t min = sizeof(void*);
        std::size_t size = align_up(block_size < min ? min : block_size, alignof(void*));
        return size;
    }

public:
    explicit FixedSizePool(std::size_t block_size, std::size_t blocks_per_page = 1024): 
        blocks_per_page_(blocks_per_page), free_list_(nullptr){
            block_size_ = adjust_block_size(block_size);
        }

    ~FixedSizePool(){
        for(auto* page : pages){
            ::operator delete[](page);
        }
    }

    void* allocate(){
        if(free_list_ == nullptr){
            expand();
        }
        Node* node = free_list_;
        free_list_ = free_list_->next;
        return static_cast<void*>(node);
    }

    void deallocate(void* ptr){
        if(ptr == nullptr){
            return;
        }
        Node* node = static_cast<Node*>(ptr);
        node->next = free_list_;
        free_list_ = node;
    }

    std::size_t block_size() const {return block_size_;}
    std::size_t blocks_per_page() const {return blocks_per_page_;}
};

struct Particle{
    float x, y, z;
    int life;
    void update(){
        life++;
    }
    static void* operator new(std::size_t n);
    static void operator delete(void* p) noexcept;
};

static FixedSizePool Particle_pool(sizeof(Particle), 4096);
void* Particle::operator new(std::size_t n){
    return Particle_pool.allocate();
}
void Particle::operator delete(void* p) noexcept{
    Particle_pool.deallocate(p);
}

int main(){
    std::vector<Particle*> vec;
    vec.reserve(100000);

    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < 100000; ++i){
        Particle* p = new Particle;
        vec.push_back(p);
    }
    for(auto* p : vec){
        delete p;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time taken: " << duration.count() << " microseconds" << std::endl;

    return 0;

}
