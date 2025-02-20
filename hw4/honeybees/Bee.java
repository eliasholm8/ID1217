package hw4.honeybees;
import java.util.Random;

public class Bee implements Runnable {
    private final Honeypot honeypot;
    private final int id;
    private final Random random = new Random();

    public Bee(Honeypot honeypot, int id) {
        this.honeypot = honeypot;
        this.id = id;
    }

    public void run() {
        while (true) {
            try {
                honeypot.addHoney(id);
                Thread.sleep(random.nextInt(1000));
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

}
