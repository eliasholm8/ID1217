package hw4.hungrybirds;


//
public class Dish {
    private int wormCount;
    private final int wormsToAdd;

    public Dish(int wormsToAdd, int initialWormCount) {
        this.wormsToAdd = wormsToAdd;
        this.wormCount = initialWormCount;
    }

    public synchronized void eatWorm(int id) throws InterruptedException {
        while (wormCount == 0) {
            wait();
        }
        System.out.println("Bird #" + id + " ate a worm. There are " + --wormCount + " worms left.");

        if (wormCount == 0) {
            System.out.println("WE NEED MORE FOOD!");
            notifyAll();
        }
    }
    public synchronized void replenishWorms() throws InterruptedException {
        while (wormCount > 0) {
            wait();
        }
        wormCount += wormsToAdd;
        System.out.println("[Parent] Added" + wormsToAdd + "worms to the count. Now:" + wormCount);
        notifyAll();
    }
}
