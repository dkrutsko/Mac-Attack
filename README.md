# Mac Attack

<p align="justify">This application implements the onion routing protocol over a wireless Ad-Hoc network using a peer-to-peer architecture.  Each instance of this application is a node in the network that is able to discover other nodes and be discovered through a neighbor discovery protocol.  Each node must have an identity that is compatible with other nodes in the network, that is, an identity must be signed by a common node called the certification node. Identities can be created and signed using this application.</p>

### Running
```bash
$ make
$ sudo ./MacAttack
```

### Requires
* pthreads
* PolarSSL (www.polarssl.org)

### Commands
```
$ MacAttack -Create [Key Length] [Filename ...]
$ MacAttack -Info   [Identity]
$ MacAttack -Sign   [Authority] [Filename ...]
$ MacAttack -Join   [Interface] [Identity] (Ignore List)
```

**WARNING:** Wildcards are not supported

### Authors
**D. Krutsko**

* Email: <dave@krutsko.net>
* Home: [dave.krutsko.net](http://dave.krutsko.net)
* GitHub: [github.com/dkrutsko](https://github.com/dkrutsko)

**S. Schneider**

* Email: <seb.schneider84@gmail.com>
* GitHub: [github.com/Harrold](https://github.com/Harrold)

**A. Shukla**

* Email: <abstronomer@gmail.com>
* GitHub: [github.com/AbsMechanik](https://github.com/AbsMechanik)