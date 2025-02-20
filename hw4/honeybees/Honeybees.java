package hw4.honeybees;

public class Honeybees {
    public static void main(String[] args) {
        Honeypot honeypot = new Honeypot(0, 10);
        Thread bear = new Thread(new Bear(honeypot));
        bear.start();
        Thread[] bees = new Thread[5];
        for (int i = 0; i < bees.length; i++) {
            bees[i] = new Thread(new Bee(honeypot, i));
            bees[i].start();
        }
    }
}
