template<typename T>
void LinkedList<T>::removeDuplicates() {
    Node<T> * current = head;
    while (current) {
        Node<T> * forward_head = current->next;
        Node<T> * forward_tail = current;
        while (forward_head) {
            if (forward_head->data == current->data) {
                // duplicated item found.
                forward_tail->next = forward_head->next;
            }
            else
                forward_tail = forward_tail->next;
            forward_head = forward_head->next;
        }
        current = current->next;
    }
}