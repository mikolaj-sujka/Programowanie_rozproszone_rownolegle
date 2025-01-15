import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.*;

public class ParallelExplorer implements Explorer {
    private ThreadsFactory threadsFactory;
    private Table2D table;
    private volatile int targetSum;
    private final AtomicBoolean started = new AtomicBoolean(false);
    private CountDownLatch readersDoneLatch;
    private final ConcurrentMap<Position2D, Boolean> visited = new ConcurrentHashMap<>();
    private final ConcurrentMap<Position2D, Integer> readValues = new ConcurrentHashMap<>();
    private final BlockingQueue<Pair> pairsToZero = new LinkedBlockingQueue<>();
    private final Set<Pair> foundPairs = ConcurrentHashMap.newKeySet();
    private final AtomicBoolean finished = new AtomicBoolean(false);
    private Thread writerThread;
    private final List<Thread> readerThreads = new ArrayList<>();
    private static final int[] DIR_COL = {-1, -1, -1, 0, 0, 1, 1, 1};
    private static final int[] DIR_ROW = {-1, 0, 1, -1, 1, -1, 0, 1};

    @Override
    public void setThreadsFactory(ThreadsFactory factory) {
        this.threadsFactory = factory;
    }

    @Override
    public void setTable(Table2D table) {
        this.table = table;
    }

    @Override
    public void start(int sum) {
        this.targetSum = sum;
        if (!started.compareAndSet(false, true)) {
            return;
        }
        int readersCount = threadsFactory.readersThreads();
        this.readersDoneLatch = new CountDownLatch(readersCount);
        writerThread = threadsFactory.writterThread(new WriterTask());
        writerThread.start();
        for (int i = 0; i < readersCount; i++) {
            ReaderTask rt = new ReaderTask();
            ThreadAndPosition tap = threadsFactory.readerThread(rt);
            if (tap == null) {
                break;
            }
            rt.setStartPosition(tap.position());
            readerThreads.add(tap.thread());
            tap.thread().start();
        }
    }

    @Override
    public Set<Pair> result() {
        if (!finished.get()) {
            return Set.of();
        }
        return foundPairs;
    }

    private class WriterTask implements Runnable {
        @Override
        public void run() {
            try {
                while (true) {
                    Pair pair = pairsToZero.take();
                    if (pair.first() == null && pair.second() == null) {
                        break;
                    }
                    table.set0(pair.first());
                    table.set0(pair.second());
                    foundPairs.add(pair);
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            } finally {
                finished.set(true);
            }
        }
    }

    private class ReaderTask implements Runnable {
        private final Queue<Position2D> queue = new ArrayDeque<>();
        private Position2D startPos;

        public void setStartPosition(Position2D pos) {
            this.startPos = pos;
        }

        @Override
        public void run() {
            try {
                if (startPos != null) {
                    queue.offer(startPos);
                }
                while (!queue.isEmpty()) {
                    Position2D current = queue.poll();
                    if (visited.putIfAbsent(current, Boolean.TRUE) != null) {
                        continue;
                    }
                    int val = table.get(current);
                    readValues.put(current, val);
                    for (Position2D neighbor : getNeighbors(current)) {
                        Integer neighborVal = readValues.get(neighbor);
                        if (neighborVal != null && val + neighborVal == targetSum) {
                            try {
                                pairsToZero.put(new Pair(current, neighbor));
                            } catch (InterruptedException e) {
                                Thread.currentThread().interrupt();
                            }
                        }
                    }
                    for (Position2D neighbor : getNeighbors(current)) {
                        if (!visited.containsKey(neighbor)) {
                            queue.offer(neighbor);
                        }
                    }
                }
            } finally {
                readersDoneLatch.countDown();
                if (readersDoneLatch.getCount() == 0) {
                    try {
                        pairsToZero.put(new Pair(null, null));
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }
    }

    private List<Position2D> getNeighbors(Position2D pos) {
        List<Position2D> result = new ArrayList<>(8);
        for (int i = 0; i < 8; i++) {
            int nc = pos.col() + DIR_COL[i];
            int nr = pos.row() + DIR_ROW[i];
            if (nc >= 0 && nc < table.cols() && nr >= 0 && nr < table.rows()) {
                result.add(new Position2D(nc, nr));
            }
        }
        return result;
    }
}
