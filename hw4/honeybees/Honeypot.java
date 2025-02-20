package hw4.honeybees;

public class Honeypot {
    private int honey;
    private int capacity;

    public Honeypot(int initialHoney, int capacity) {
        this.honey = initialHoney;
        this.capacity = capacity;
    }

    public synchronized void addHoney(int id) throws InterruptedException {
        while (honey == capacity) {
            wait();
        }
        honey++;
        System.out.println("Bee #" + id + " added to the pot. Now: " + honey);
        notifyAll();
    }
    public synchronized void takeHoney() throws InterruptedException {
        while (honey < capacity) {
            wait();
        }
        honey = 0;
        System.out.println("[Bear] Took all the honey. Now: " + honey);
        notifyAll();
    }
}
