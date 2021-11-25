/************************************************************
  main.h file
  Includes struct definitions and queue methods
 ************************************************************/


typedef struct {      //  Thread Structure
  int id; 
  ucontext_t ucont ;  // to store context 
  void* retval;       // to store return value of thread
  int total_remaining_time;
  int* burst_sequence;
} Thread ;


struct ThreadInfo {
ucontext_t* context;
int state;
} ;


typedef struct node { // queue node
  struct node *next;
  Thread  *data;
} node; 

typedef struct queue { // queue
  struct node *front;
  struct node *rear;
  int count;
} queue; 

struct sigaction sa;  // signal sa
struct ThreadInfo threadArray[6];  // input is accepted until 5 threads


void queue_init (queue *que) {
  que->front = que->rear = NULL;
  que->count = 0;
}


int queue_size (queue *que) {      //Returns queue size
        return que->count;
}


int enqueue (queue *que, Thread  *thread) { // adds new element
        node *temp = (node *) malloc(sizeof(node));
        if (temp == NULL) {
    return 0;
  }
  temp->data = thread;
  temp->next = NULL;

  if (que->rear) {
    que->rear->next = temp;
    que->rear = temp;
  }
  else
  {
    que->rear = temp;
    que->front = temp;
  }
  ++(que->count);

  return 1;
}


Thread  * dequeue (queue *que) {      //Removes a node from front of queue
  if (que->front == NULL) return NULL;
  node *temp = (node *) malloc(sizeof(node));
  temp = que->front;
  que->front = que->front->next;
  Thread  *ret = NULL;
  ret = temp->data;
  free(temp);
  --(que->count);
  return ret;
}

/*Prints elements of a queue*/
void print_queue (queue *que) {

  node *temp = que->front;
  if (temp == NULL) {
    printf(" ");
    return;
  }
  while (temp) {
    printf("T%d,", temp->data->id);
    temp = temp->next;
  }
}

node* get_node(int index, queue* queue)
{
  node* node_ = queue -> front;
  int counter = 0;
  while (counter != index)
  {
    node_ = node_->next;
    counter ++;
  }
  return node_;
}

Thread* remove_node_by_idx(int index, queue* queue)
{
  if (index == 0)
  {
    Thread* data_to_remove = queue -> front -> data;
    if (queue -> front == queue -> rear)
    {
      free(queue -> front);
      queue -> front = NULL;
      queue -> rear = NULL;
    }
    else
    {
      node* front_node_of_queue = queue -> front;
      queue -> front = queue -> front -> next;
      free(front_node_of_queue);
    }
    return data_to_remove;
  }
  else
  {
    node* parent = NULL;
    node* selected_node = queue -> front;
    node* child = selected_node -> next;
    Thread* data_to_remove;
    int pos = 0;
    while(pos < index)
    {
      parent = selected_node;
      selected_node = child;
      child =  child -> next;
      pos++;
    }
    if (!child)
    {
      parent -> next = NULL;
      queue -> rear = parent;
    }
    else
    {
      parent -> next = selected_node -> next;
    }
    data_to_remove = selected_node -> data;
    selected_node -> next = NULL;
    free(selected_node);
    return data_to_remove;
  }
}

int minimun_in_array(int array[], int lenght)
{
    int min_in_array = array[0];
    for (int index = 0; index < lenght; index++)
    {
      if (array[index] < min_in_array)
      {
        min_in_array = array[index];
      }
    }
    return min_in_array;
  }
