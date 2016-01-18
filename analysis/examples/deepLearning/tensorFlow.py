#!/usr/bin/env python

import math
import sys
import os
import time

import dpf_import

import numpy as np
import tensorflow.python.platform
import tensorflow as tf


flags = tf.app.flags
FLAGS = flags.FLAGS
flags.DEFINE_float('learning_rate', 0.01, 'Initial learning rate.')
flags.DEFINE_integer('hidden1', 128, 'Number of units in hidden layer 1.')
flags.DEFINE_integer('hidden2', 128, 'Number of units in hidden layer 2.')
flags.DEFINE_integer('hidden3', 128, 'Number of units in hidden layer 3.')
flags.DEFINE_integer('hidden4', 128, 'Number of units in hidden layer 4.')
flags.DEFINE_integer('max_steps', 2000, 'Number of steps to run trainer.')
flags.DEFINE_integer('batch_size', 100, 'Batch size. Must divide evenly into the dataset sizes.')
flags.DEFINE_string('filename', 'signal.dpf', 'Filename.')

NTRACKS = 10
TRACKSIZE = 12
SIZE = NTRACKS*TRACKSIZE


def inference_model(x):
    """ Inference model """
    def layer(x, shape, name, unit=tf.nn.relu):
        with tf.name_scope(name) as scope:
            weights = tf.Variable(tf.truncated_normal(shape, stddev=1.0 / math.sqrt(float(shape[0]))), name='weights')
            biases = tf.Variable(tf.constant(0.0, shape=[shape[1]]), name='biases')
            layer = unit(tf.matmul(x, weights) + biases)
        return layer

    hidden1 = layer(x, [SIZE, FLAGS.hidden1], 'hidden1')
    hidden2 = layer(hidden1, [FLAGS.hidden1, FLAGS.hidden2], 'hidden2')
    hidden3 = layer(hidden2, [FLAGS.hidden2, FLAGS.hidden3], 'hidden3')
    hidden4 = layer(hidden2, [FLAGS.hidden3, FLAGS.hidden4], 'hidden4')
    output = layer(hidden3, [FLAGS.hidden4, 2], 'softmax', unit=tf.nn.softmax)
    return output


def loss_model(y, p):
    """ Loss model """
    loss = -tf.reduce_sum(tf.cast(y, tf.float32)*tf.log(p+0.001))
    return loss


def training_operation(loss):
    """ Training operation """
    tf.scalar_summary(loss.op.name, loss)
    optimizer = tf.train.GradientDescentOptimizer(FLAGS.learning_rate)
    global_step = tf.Variable(0, name='global_step', trainable=False)
    train_op = optimizer.minimize(loss, global_step=global_step)
    return train_op


def evaluation_operation(y, p):
    """ Evaluation operation """
    correct_prediction = tf.equal(tf.argmax(y, 1), tf.argmax(p, 1))
    accuracy = tf.reduce_mean(tf.cast(correct_prediction, "float"))
    return accuracy


def main(_):

    with tf.Graph().as_default():
        x = tf.placeholder(tf.float32, [None, SIZE])
        y = tf.placeholder(tf.int32, [None, 2])
        p = inference_model(x)
        loss = loss_model(y, p)
        training_op = training_operation(loss)
        accuracy = evaluation_operation(y, p)
        summary_op = tf.merge_all_summaries()
        saver = tf.train.Saver()

        with tf.Session() as sess:
            init = tf.initialize_all_variables()
            sess.run(init)
            summary_writer = tf.train.SummaryWriter('.', graph_def=sess.graph_def)
            batch = batch_generator(FLAGS.batch_size)

            for step in range(FLAGS.max_steps):
                start_time = time.time()

                batch_xs, batch_ys = next(batch)
                feed_dict = {x: batch_xs, y: batch_ys}

                sess.run([training_op, loss], feed_dict=feed_dict)
                _, loss_value = sess.run([training_op, loss], feed_dict=feed_dict)

                duration = time.time() - start_time
                if step % 100 == 0:
                    print('Step %d: loss = %.2f (%.3f sec)' % (step, loss_value, duration))
                    summary_str = sess.run(summary_op, feed_dict=feed_dict)
                    summary_writer.add_summary(summary_str, step)

                if (step + 1) % 1000 == 0 or (step + 1) == FLAGS.max_steps:
                    saver.save(sess, '.', global_step=step)

                prediction = sess.run(accuracy, feed_dict=feed_dict)
                print('Current accuracy {:.4f}'.format(prediction))


def batch_generator(batch_size):
    x = np.zeros((batch_size, SIZE), dtype=np.float32)
    y = np.zeros((batch_size, 2), dtype=np.int32)

    i = 0
    with open(FLAGS.filename, 'rb') as f:
        while True:
            event = dpf_import.read(f)
            if event is None:
                f.seek(0, 0)
                continue
            t = np.zeros(SIZE, dtype=np.float32)
            for j, track in enumerate(sorted(event.tracks, key=lambda x: x.px**2 + x.py**2 + x.pz**2)[:NTRACKS]):
                t[j * TRACKSIZE + 0] = float(track.charge)
                t[j * TRACKSIZE + 1] = float(track.prob)
                t[j * TRACKSIZE + 2] = track.px
                t[j * TRACKSIZE + 3] = track.py
                t[j * TRACKSIZE + 4] = track.pz
                t[j * TRACKSIZE + 5] = track.eid
                t[j * TRACKSIZE + 6] = track.muid
                t[j * TRACKSIZE + 7] = track.kid
                t[j * TRACKSIZE + 8] = track.prid
                t[j * TRACKSIZE + 9] = track.x
                t[j * TRACKSIZE + 10] = track.y
                t[j * TRACKSIZE + 11] = track.z
            if not np.all(np.isfinite(t)):
                print(t)
            x[i] = t
            y[i] = np.array([event.header.isSignal, 1-event.header.isSignal], dtype=np.int32)
            i += 1
            if i == batch_size:
                i = 0
                yield x, y


if __name__ == '__main__':
    tf.app.run()
