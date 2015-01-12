# biombo
Simple encrypted picture viewer for KDE desktop.

With this application you can visualize encrypted (OpenPGP) pictures without the need to have them decrypted on disk first. This can prevent, for example, the creation of thumbnails of your pictures by a file manager, that coul leak information in them. It can prevent the backup of the unencrypted picture by a backup service or the risk of having them decrypted on disk.

Uses SecureArray from the Qt Cryptographic Architecture (QCA) to provide an array of memory from a pool that is, at least partly, secure. It means that the contents of the memory should not be made available to other applications. By comparison, a QByteArray or QString may be held in pages that might be swapped to disk or free'd without being cleared first.

The code used to interact with GPGme is from the Basket Note Pads application: https://github.com/kelvie/basket/tree/master.
