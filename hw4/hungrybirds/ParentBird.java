package hw4.hungrybirds;
import java.util.Random;

public class ParentBird implements Runnable {
    private final Dish dish;
    private final Random random = new Random();

    public ParentBird (Dish dish) {
        this.dish = dish;
    }

    public void run() {
        while (true) {
            try {
                dish.replenishWorms();
                Thread.sleep(random.nextInt(1000));
            }
            catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
