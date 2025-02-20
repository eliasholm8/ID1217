package hw4.hungrybirds;

import java.util.Random;

public class BabyBird implements Runnable {
    private int id;
    private final Dish dish;
    private final Random random = new Random();



    public BabyBird(int id, Dish dish) {
        this.id = id;
        this.dish = dish;
    }

    public void run() {
        while (true) {
            try {
                dish.eatWorm(id);
                Thread.sleep(random.nextInt(1000));
            }
            catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
