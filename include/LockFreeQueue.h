#ifndef LOCKFREEQUEUE_H
#define LOCKFREEQUEUE_H
//无锁队列实现
#include <atomic>
#include <memory>
#include <queue>

template <typename T>
class LockFreeQueue
{
    private:
        struct Node
        {
            std::shared_ptr<T> data;
            //这里使用了std::atomic类型的next指针来保证多线程环境下的线程安全
            std::atomic<Node*> next;
            //构造函数,使用std::make_shared来构造data,并使用移动语义来传递value
            Node(T value) : data(std::make_shared<T>(std::move(value)))next(nullptr) {};
            Node() : next(nullptr) {};
        };
        std::atomic<Node*> head;
        std::atomic<Node*> tail;
    public:
        LockFreeQueue():head(new Node()),tail(head.load()){}
        ~LockFreeQueue(){
            while(Node* const old_head = head.load()){
                head.store(old_head->next);
                delete old_head;
            }
        }
        //入队操作
        void push(T value){
            Node* new_node = new Node(std::move(value));
            Node* old_tail = tail.load();
            //这里使用了自旋锁来保证多线程环境下的线程安全
            while(true){
                Node* next = old_tail->next.load();
                //这里使用了CAS操作来保证多线程环境下的线程安全
                if(!next){
                    //next为空,说明当前节点是队列的最后一个节点
                    if(old_tail->next.compare_exchange_weak(next,new_node)){
                        tail.compare_exchange_weak(old_tail,new_node);
                        return;
                    }
                }else{
                    //next不为空,说明当前节点不是队列的最后一个节点
                    tail.compare_exchange_weak(old_tail,next);
                }
            }
        }
        //出队操作
        std::shared_ptr<T> pop(){
            Node* old_head = head.load();
            //这里使用了自旋锁来保证多线程环境下的线程安全
            while(true){
                Node* new_head = old_head->next.load();
                if(!new_head){
                    return nullptr;
                }
                //这里使用了CAS操作来保证多线程环境下的线程安全
                if(head.compare_exchange_weak(old_head,new_head)){
                    std::shared_ptr<T> res = new_head->data;
                    delete old_head;
                    return res;
                }
            }
        }
};

#endif // LOCKFREEQUEUE_H