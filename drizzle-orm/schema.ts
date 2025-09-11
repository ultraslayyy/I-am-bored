import { boolean, customType, index, pgEnum, pgTable, text, timestamp, uniqueIndex, uuid } from './types/pg-core';
import { sql } from './types';

export const themeOptions = pgEnum('theme', ['light', 'dark', 'system']);
export const mfaMethods = pgEnum('mfa_methods', ['otp', 'webauthn']);
export const rcptTypes = pgEnum('rcpt_types', [
    'to',
    'cc',
    'bcc'
]);
export const mailboxTypes = pgEnum('mailbox_types', [
    'inbox',
    'sent',
    'draft',
    'trash'
]);

const tsvector = customType<{ data: string; notNull: false, default: false; }>({
    dataType() {
        return 'tsvector';
    }
});

export const users = pgTable('users', {
    id: uuid('id').defaultRandom().primaryKey(),
    username: text('username').notNull().unique(),
    email: text('email').notNull().unique(),
    passwordHash: text('password_hash').notNull(),
    createdAt: timestamp('created_at').defaultNow(),
    updatedAt: timestamp('updated_at').defaultNow(),
    deletedAt: timestamp('deleted_at'),
    isActive: boolean('is_active').default(false),
    lastLoginAt: timestamp('last_login_at')
}, (table) => [
    uniqueIndex('users_username_idx').on(table.username),
    uniqueIndex('users_email_idx').on(table.email),
    index('active_users_idx').on(table.username).where(sql`deleted_at IS NULL AND is_active = true`)
]);

export const userSettings = pgTable('user_settings', {
    userId: uuid('user_id').primaryKey().references(() => users.id, { onDelete: 'cascade' }),
    theme: themeOptions('theme').default('light').notNull(),
    language: text('language').default('en').notNull(),
    mfaEnabled: boolean('mfa_enabled').default(false).notNull(),
    mfaMethods: mfaMethods().array()
});

export const mailboxes = pgTable('mailboxes', {
    id: uuid('id').defaultRandom().primaryKey(),
    userId: uuid('user_id').references(() => users.id, { onDelete: 'cascade' }).notNull(),
    name: text('name').notNull(),
    mailboxType: mailboxTypes('mailbox_type').default('inbox'),
    systemMailbox: boolean('system_mailbox').default(false),
    createdAt: timestamp('created_at').defaultNow(),
    updatedAt: timestamp('updated_at').defaultNow(),
    deletedAt: timestamp('deleted_at')
}, (table) => [
    index('mailbox_user_idx').on(table.userId),
    uniqueIndex('mailbox_user_name_idx').on(table.userId, table.name)
]);