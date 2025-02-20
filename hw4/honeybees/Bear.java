package hw4.honeybees;
import java.util.Random;

public class Bear implements Runnable {
    Honeypot honeypot;
    Random random = new Random();

    public Bear(Honeypot honeypot) {
        this.honeypot = honeypot;
    }

    public void run() {
        while (true) {
            try {
                honeypot.takeHoney();
                Thread.sleep(random.nextInt(1000));
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
