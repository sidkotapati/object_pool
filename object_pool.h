/// BRIEF: SIMPLE OBJECT POOL IMPL, NO AI GENERATED CODE

/// NOTE: Assumes objects construct themselves.

template <typename T>
class ObjectPool {
private:
    struct ObjectNode {
        ObjectNode* next_obj{ nullptr };
        T obj;
    };
    struct ObjectBatch {
        ObjectBatch* next_batch{ nullptr };
    };

    [[no_unique_address]] std::allocator<ObjectNode> batch_allocator_;
    static constexpr size_t BATCH_SIZE{ 64 }; 
    ObjectBatch* batch_stack_{ nullptr };
    ObjectNode* head_{ nullptr };

    void allocate_batch() {
        // Allocate new batch (arena), need batch pointer to add to batch stack
        // and node at the start of the batch
        ObjectNode* alloc_begin = batch_allocator_.allocate(BATCH_SIZE + 1);
        ObjectNode* batch_start_node = alloc_begin + 1;
        ObjectBatch* new_batch = reinterpret_cast<ObjectBatch*>(alloc_begin);

        // Attach new arena to batch stack
        new_batch->next_batch = batch_stack_;
        batch_stack_ = new_batch;
        
        // Chain together nodes in batch and add them to head
        ObjectNode* prev_head = head_;
        ObjectNode* curr_node = batch_start_node;
        for (size_t i{ 1 }; i < BATCH_SIZE; ++i) {
            curr_node->next_obj = curr_node + 1;
            curr_node = curr_node->next_obj;
        }
        curr_node->next_obj = prev_head;
        head_ = batch_start_node;
    }

public:
    // Shallow copies will double free, need to delete;
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

    ~ObjectPool() {
        while (batch_stack_) {
            ObjectBatch* to_delete_batch = batch_stack_;
            batch_stack_ = batch_stack_->next_batch;

            ObjectNode* batch_start_node = reinterpret_cast<ObjectNode*>(to_delete_batch);

            batch_allocator_.deallocate(batch_start_node, BATCH_SIZE + 1);
        }
    }

    T* allocate() {
        if (!head_) allocate_batch();
        
        ObjectNode* to_return = head_;
        head_ = head_->next_obj;
        return &to_return->obj;
    }

    void deallocate(T* obj) {
        // Use some pointer and byte offset magic to get use a pointer of type ObjectNode*
        // that we can add back to the head_ stack (essentially free objects).
        std::byte* obj_begin = reinterpret_cast<std::byte*>(obj);

        std::byte* node_begin = obj_begin - sizeof(ObjectNode*);

        ObjectNode* freed_node = reinterpret_cast<ObjectNode*>(node_begin);

        freed_node->next_obj = head_;
        head_ = freed_node;
    }
};
